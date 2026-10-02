#include "error.h"
#include "analyzer.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <ReadBarcode.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <stdexcept>

namespace qr
{
cv::Mat loadImage(const std::filesystem::path &path)
{
#ifdef _WIN32
    std::unique_ptr<FILE, decltype(&fclose)> file(_wfopen(path.c_str(), L"rb"), fclose);
#else
    std::unique_ptr<FILE, decltype(&fclose)> file(fopen(path.c_str(), "rb"), fclose);
#endif
    if (!file)
        throw qr::Error("error.file_open");
    std::vector<unsigned char> bytes;
    std::array<unsigned char, 65536> block{};
    for (;;) {
        const auto count = fread(block.data(), 1, block.size(), file.get());
        if (bytes.size() + count > 256u * 1024u * 1024u)
            throw qr::Error("error.file_size");
        bytes.insert(bytes.end(), block.begin(), block.begin() + count);
        if (count != block.size()) {
            if (ferror(file.get()))
                throw qr::Error("error.file_read");
            break;
        }
    }
    if (bytes.empty())
        throw qr::Error("error.image");
    // Decode unchanged so alpha can be composited on white, not silently turned black.
    cv::Mat image = cv::imdecode(bytes, cv::IMREAD_UNCHANGED);
    if (image.empty())
        throw qr::Error("error.image");
    return image;
}

cv::Mat prepareImage(const cv::Mat &image)
{
    if (image.empty())
        throw qr::Error("error.image");
    if (image.channels() != 1 && image.channels() != 3 && image.channels() != 4)
        throw qr::Error("error.image");
    cv::Mat pixels;
    if (image.depth() == CV_16U)
        image.convertTo(pixels, CV_8U, 1.0 / 257.0);
    else if (image.depth() == CV_8U)
        pixels = image;
    else
        throw qr::Error("error.image");

    const auto size = analysisSize({pixels.cols, pixels.rows});
    cv::Mat small;
    if (size.width != pixels.cols || size.height != pixels.rows)
        cv::resize(pixels, small, {size.width, size.height}, 0, 0, cv::INTER_AREA);
    else
        small = pixels;

    cv::Mat gray;
    if (small.channels() == 4) {
        cv::Mat bgr(small.rows, small.cols, CV_8UC3);
        for (int y = 0; y < small.rows; ++y) {
            const auto *src = small.ptr<cv::Vec4b>(y);
            auto *dst = bgr.ptr<cv::Vec3b>(y);
            for (int x = 0; x < small.cols; ++x)
                for (int c = 0; c < 3; ++c)
                    dst[x][c] = static_cast<unsigned char>(
                        (src[x][c] * src[x][3] + 255 * (255 - src[x][3]) + 127) / 255);
        }
        cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    } else if (small.channels() == 3)
        cv::cvtColor(small, gray, cv::COLOR_BGR2GRAY);
    else
        gray = small;
    return gray;
}

static ZXing::Result readQR(const cv::Mat &image)
{
    ZXing::DecodeHints hints;
    hints.setFormats(ZXing::BarcodeFormat::QRCode).setTryHarder(true).setTryRotate(true);
    return ZXing::ReadBarcode(
        {image.data, image.cols, image.rows, ZXing::ImageFormat::Lum, static_cast<int>(image.step)},
        hints);
}

static bool decodeOriginalRegion(const cv::Mat &gray, Code &code)
{
    // Multi-code localization can give slightly imperfect corners. Warping a
    // small QR then resamples its modules a second time and may destroy data.
    // Let ZXing sample the original pixels before attempting rectification.
    const std::vector<cv::Point2f> candidate(code.corners.begin(), code.corners.end());
    const auto bounds = cv::boundingRect(candidate) & cv::Rect(0, 0, gray.cols, gray.rows);
    if (bounds.empty())
        return false;
    const int margin = std::max(12, std::max(bounds.width, bounds.height) / 6);
    const auto region = cv::Rect(bounds.x - margin, bounds.y - margin, bounds.width + 2 * margin,
                                 bounds.height + 2 * margin) &
                        cv::Rect(0, 0, gray.cols, gray.rows);
    // Keep a known white quiet zone even when the detected symbol touches the
    // source edge. Never resize or threshold the source pixels on this path.
    constexpr int pad = 16;
    cv::Mat patch;
    cv::copyMakeBorder(gray(region), patch, pad, pad, pad, pad,
                       cv::BORDER_CONSTANT | cv::BORDER_ISOLATED, cv::Scalar(255));
    ZXing::DecodeHints hints;
    hints.setFormats(ZXing::BarcodeFormat::QRCode).setTryHarder(true).setTryRotate(true);
    const auto results = ZXing::ReadBarcodes(
        {patch.data, patch.cols, patch.rows, ZXing::ImageFormat::Lum, static_cast<int>(patch.step)},
        hints);
    const double area = std::abs(cv::contourArea(candidate));
    const ZXing::Result *best = nullptr;
    float bestOverlap = 0.5f;
    for (const auto &result : results) {
        if (!result.isValid())
            continue;
        std::vector<cv::Point2f> position;
        for (const auto &point : result.position())
            position.emplace_back(float(point.x + region.x - pad), float(point.y + region.y - pad));
        const double decodedArea = std::abs(cv::contourArea(position));
        if (area <= 0 || decodedArea <= 0)
            continue;
        std::vector<cv::Point2f> intersection;
        const float overlap = cv::intersectConvexConvex(candidate, position, intersection);
        const float match = overlap / static_cast<float>(std::max(area, decodedArea));
        // A crop can also contain a neighbor. Match the returned quadrilateral
        // to THIS OpenCV candidate instead of using the first decoded payload.
        if (match > bestOverlap) {
            bestOverlap = match;
            best = &result;
        }
    }
    if (!best)
        return false;
    code.decoded = true;
    code.text = best->text();
    return true;
}

static void decode(const cv::Mat &gray, Code &code)
{
    if (decodeOriginalRegion(gray, code))
        return;
    // OpenCV's four corners describe the symbol without its quiet zone.
    // Rectify ONLY this candidate, then add a white quiet zone for ZXing.
    double longest = 0;
    for (int i = 0; i < 4; ++i)
        longest = std::max(longest, cv::norm(code.corners[i] - code.corners[(i + 1) % 4]));
    const int side = std::clamp(static_cast<int>(std::ceil(longest)), 64, 1600);
    const int pad = std::max(16, side / 8);
    const float start = static_cast<float>(pad), end = static_cast<float>(pad + side - 1);
    const std::array<cv::Point2f, 4> dst{{{start, start}, {end, start}, {end, end}, {start, end}}};
    const auto transform = cv::getPerspectiveTransform(code.corners.data(), dst.data());
    cv::Mat patch;
    cv::warpPerspective(gray, patch, transform, {side + 2 * pad, side + 2 * pad}, cv::INTER_NEAREST,
                        cv::BORDER_CONSTANT, cv::Scalar(255));
    // Do not allow adjacent codes in the source to intrude on the new quiet zone.
    patch.rowRange(0, pad).setTo(255);
    patch.rowRange(pad + side, patch.rows).setTo(255);
    patch.colRange(0, pad).setTo(255);
    patch.colRange(pad + side, patch.cols).setTo(255);
    auto result = readQR(patch);
    if (!result.isValid()) {
        cv::threshold(patch, patch, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        result = readQR(patch);
    }
    if (result.isValid()) {
        code.decoded = true;
        code.text = result.text();
    }
}

static std::vector<Code> detectCandidates(const cv::Mat &gray)
{
    // QRCodeDetector uses k-means++ with OpenCV's per-thread RNG. Isolate the
    // seed from earlier analyses and restore the caller's state on every exit.
    struct DetectionRandomState {
        cv::RNG &rng = cv::theRNG();
        uint64_t saved = rng.state;
        DetectionRandomState()
        {
            rng.state = 0xffffffff;
        }
        ~DetectionRandomState()
        {
            rng.state = saved;
        }
    } randomState;
    std::vector<Code> codes;
    cv::QRCodeDetector detector;
    std::vector<cv::Point2f> corners;
    detector.detectMulti(gray, corners);
    if (corners.empty())
        detector.detect(gray, corners);
    for (size_t i = 0; i + 3 < corners.size(); i += 4) {
        Code code;
        std::copy_n(corners.begin() + i, 4, code.corners.begin());
        bool finite = true;
        for (const auto &p : code.corners)
            finite = finite && std::isfinite(p.x) && std::isfinite(p.y);
        const std::vector<cv::Point2f> polygon(code.corners.begin(), code.corners.end());
        if (!finite || !cv::isContourConvex(polygon) || std::abs(cv::contourArea(polygon)) < 4)
            continue;
        codes.push_back(std::move(code));
    }
    return codes;
}

static bool samePosition(const Code &a, const Code &b)
{
    const std::vector<cv::Point2f> first(a.corners.begin(), a.corners.end());
    const std::vector<cv::Point2f> second(b.corners.begin(), b.corners.end());
    std::vector<cv::Point2f> intersection;
    const double area =
        std::max(std::abs(cv::contourArea(first)), std::abs(cv::contourArea(second)));
    return area > 0 && cv::intersectConvexConvex(first, second, intersection) > area * 0.5;
}

static std::vector<cv::Rect> compactRegions(const cv::Mat &gray)
{
    // Join nearby dark QR modules to propose compact crops. RETR_LIST also
    // finds codes inside dark browser/window frames; proposals are NOT results.
    // Every proposal must still pass OpenCV QRCodeDetector before ZXing reads it.
    cv::Mat binary;
    cv::threshold(gray, binary, 160, 255, cv::THRESH_BINARY_INV);
    cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, cv::Mat::ones(3, 3, CV_8U));
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    std::vector<cv::Rect> regions;
    for (const auto &contour : contours) {
        const auto bounds = cv::boundingRect(contour);
        if (bounds.width < 36 || bounds.height < 36 || bounds.width > 256 || bounds.height > 256 ||
            bounds.width > bounds.height * 1.5 || bounds.height > bounds.width * 1.5 ||
            std::abs(cv::contourArea(contour)) < bounds.area() * 0.4)
            continue;
        const int margin = std::max(12, std::max(bounds.width, bounds.height) / 5);
        const auto region = cv::Rect(bounds.x - margin, bounds.y - margin,
                                     bounds.width + margin * 2, bounds.height + margin * 2) &
                            cv::Rect(0, 0, gray.cols, gray.rows);
        if (std::none_of(regions.begin(), regions.end(), [&](const auto &other) {
                return (region & other).area() > std::max(region.area(), other.area()) * 0.8;
            }))
            regions.push_back(region);
    }
    return regions;
}

Analysis analyze(const cv::Mat &image)
{
    const auto gray = prepareImage(image);
    Analysis analysis{{image.cols, image.rows}, {gray.cols, gray.rows}, detectCandidates(gray)};
    for (auto &code : analysis.codes)
        decode(gray, code);
    const auto scan = [&](const cv::Mat &pixels, int x, int y, double sx, double sy) {
        for (auto code : detectCandidates(pixels)) {
            Code mapped = code;
            for (auto &point : mapped.corners) {
                point.x = static_cast<float>((point.x + x) * sx);
                point.y = static_cast<float>((point.y + y) * sy);
            }
            auto existing =
                std::find_if(analysis.codes.begin(), analysis.codes.end(),
                             [&](const auto &other) { return samePosition(mapped, other); });
            if (existing != analysis.codes.end() && existing->decoded)
                continue;
            decode(pixels, code);
            mapped.decoded = code.decoded;
            mapped.text = std::move(code.text);
            if (existing == analysis.codes.end())
                analysis.codes.push_back(std::move(mapped));
            else if (mapped.decoded)
                *existing = std::move(mapped);
        }
    };
    for (const auto &region : compactRegions(gray))
        scan(gray(region), region.x, region.y, 1, 1);
    // The overview preserves the 1920x1080 limit. OpenCV additionally reduces
    // its search image to a 512-pixel short side, which can hide small symbols.
    // Overlapping original-pixel tiles emulate a manual crop without losing
    // modules to downscaling. Every tile is itself below the analysis limit.
    if (image.cols > 960 || image.rows > 720) {
        constexpr int tileSize = 512, step = 320;
        const double sx = double(gray.cols) / image.cols, sy = double(gray.rows) / image.rows;
        std::vector<cv::Rect> visited;
        for (int y = 0;; y = std::min(y + step, std::max(0, image.rows - tileSize))) {
            const int height = std::min(tileSize, image.rows - y);
            for (int x = 0;; x = std::min(x + step, std::max(0, image.cols - tileSize))) {
                const int width = std::min(tileSize, image.cols - x);
                const auto tile = prepareImage(image(cv::Rect(x, y, width, height)));
                scan(tile, x, y, sx, sy);
                for (const auto &region : compactRegions(tile)) {
                    const cv::Rect absolute(region.x + x, region.y + y, region.width,
                                            region.height);
                    if (std::any_of(visited.begin(), visited.end(), [&](const auto &other) {
                            return (absolute & other).area() >
                                   std::max(absolute.area(), other.area()) * 0.8;
                        }))
                        continue;
                    visited.push_back(absolute);
                    scan(tile(region), absolute.x, absolute.y, sx, sy);
                }
                if (x + width == image.cols)
                    break;
            }
            if (y + height == image.rows)
                break;
        }
    }
    // Stable spatial order is also the numbering used by the indicator.
    std::sort(analysis.codes.begin(), analysis.codes.end(), [](const Code &a, const Code &b) {
        const auto ca = (a.corners[0] + a.corners[2]) * 0.5f;
        const auto cb = (b.corners[0] + b.corners[2]) * 0.5f;
        return ca.y == cb.y ? ca.x < cb.x : ca.y < cb.y;
    });
    return analysis;
}
} // namespace qr
