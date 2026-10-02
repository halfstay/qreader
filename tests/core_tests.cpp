#include "analyzer.h"
#include <BitMatrix.h>
#include <MultiFormatWriter.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <exception>

static void require(bool condition, const char *message)
{
    if (!condition)
        throw std::runtime_error(message);
}
static cv::Mat symbol(const std::string &text, int size = 240)
{
    ZXing::MultiFormatWriter writer(ZXing::BarcodeFormat::QRCode);
    writer.setEncoding(ZXing::CharacterSet::UTF8).setMargin(4);
    const auto bits = writer.encode(text, size, size);
    cv::Mat image(bits.height(), bits.width(), CV_8UC1, cv::Scalar(255));
    for (int y = 0; y < bits.height(); ++y)
        for (int x = 0; x < bits.width(); ++x)
            if (bits.get(x, y))
                image.at<unsigned char>(y, x) = 0;
    return image;
}
static void expectText(const qr::Analysis &result, const std::string &text)
{
    require(std::any_of(result.codes.begin(), result.codes.end(),
                        [&](const auto &c) { return c.decoded && c.text == text; }),
            "Expected QR payload not decoded");
}
static void expectSame(const qr::Analysis &actual, const qr::Analysis &expected)
{
    require(actual.codes.size() == expected.codes.size(),
            "Repeated analysis changed detected count");
    for (size_t i = 0; i < actual.codes.size(); ++i) {
        require(actual.codes[i].decoded == expected.codes[i].decoded &&
                    actual.codes[i].text == expected.codes[i].text,
                "Repeated analysis changed decoded content");
        require(actual.codes[i].corners == expected.codes[i].corners,
                "Repeated analysis changed positions");
    }
}
int main()
{
    try {
        cv::setNumThreads(1);
        for (auto input : {qr::Size{1920, 1080},
                           {640, 480},
                           {1, 1},
                           {1921, 1080},
                           {1920, 1081},
                           {7680, 2160},
                           {800, 3000},
                           {1, 100000}}) {
            auto size = qr::analysisSize(input);
            require(size.width <= 1920 && size.height <= 1080, "Analysis exceeds limit");
            require(size.width >= 1 && size.height >= 1, "Analysis has zero extent");
            require(size.width <= input.width && size.height <= input.height,
                    "Unexpected upscaling");
            if (input.width <= 1920 && input.height <= 1080)
                require(size.width == input.width && size.height == input.height,
                        "Small image was resized");
        }
        auto fit = qr::fitInside({1920, 1080}, {10, 20, 410, 420});
        require(fit.width() == 400 && fit.height() == 225 && fit.top == 107,
                "Indicator aspect ratio");
        auto rect = qr::dragRect(300, 400, -1400, -50, {-1280, 0, 1920, 1080});
        require(rect.left == -1280 && rect.top == 0 && rect.right == 300 && rect.bottom == 400,
                "Reverse/negative monitor drag");
        std::cout << "PASS dimensions, mapping, negative screen coordinates\n";

        const std::string payload = "https://example.org/qreader?test=1";
        const auto qrImage = symbol(payload);
        const auto single = qr::analyze(qrImage);
        require(single.codes.size() == 1, "Single QR location count");
        expectText(single, payload);
        for (const auto &p : single.codes.front().corners)
            require(p.x >= 0 && p.x < qrImage.cols && p.y >= 0 && p.y < qrImage.rows,
                    "Candidate outside image");
        expectText(qr::analyze(symbol("二维码测试 · QReader")), "二维码测试 · QReader");
        auto blank = cv::Mat(300, 400, CV_8UC3, cv::Scalar(255, 255, 255));
        require(qr::analyze(blank).codes.empty(), "Blank image produces candidates");
        std::cout << "PASS single QR, UTF-8, blank image\n";

        cv::Mat multiple(600, 900, CV_8UC1, cv::Scalar(255));
        qrImage.copyTo(multiple(cv::Rect(40, 40, qrImage.cols, qrImage.rows)));
        auto second = symbol("second-code");
        second.copyTo(multiple(cv::Rect(540, 310, second.cols, second.rows)));
        const auto many = qr::analyze(multiple);
        require(many.codes.size() == 2, "Multiple QR location count");
        expectText(many, payload);
        expectText(many, "second-code");
        qrImage.copyTo(multiple(cv::Rect(540, 310, qrImage.cols, qrImage.rows)));
        const auto duplicate = qr::analyze(multiple);
        require(duplicate.codes.size() == 2 && duplicate.codes[0].decoded &&
                    duplicate.codes[1].decoded,
                "Identical payloads at different positions were lost");
        std::cout << "PASS multiple QR and duplicate payload positions\n";

        cv::Mat large;
        cv::resize(multiple, large, {3600, 2400}, 0, 0, cv::INTER_NEAREST);
        auto resized = qr::analyze(large);
        require(resized.analyzed.width == 1620 && resized.analyzed.height == 1080,
                "Large image resize");
        expectText(resized, payload);
        cv::Mat rotated;
        cv::rotate(qrImage, rotated, cv::ROTATE_90_CLOCKWISE);
        expectText(qr::analyze(rotated), payload);
        std::array<cv::Point2f, 4> src{{{0, 0}, {239, 0}, {239, 239}, {0, 239}}};
        std::array<cv::Point2f, 4> dst{{{50, 35}, {300, 60}, {270, 300}, {30, 280}}};
        cv::Mat perspective;
        cv::warpPerspective(qrImage, perspective,
                            cv::getPerspectiveTransform(src.data(), dst.data()), {340, 340},
                            cv::INTER_NEAREST, cv::BORDER_CONSTANT, cv::Scalar(255));
        expectText(qr::analyze(perspective), payload);
        std::cout << "PASS downscaling, rotation, perspective rectification\n";

        // A small symbol in a 2154x1380 window used to produce zero locations,
        // although selecting just its original pixels decoded successfully.
        const auto tiny = symbol("https://example.org/qreader?large-window=1", 90);
        cv::Mat largeWindow(1380, 2154, CV_8UC1, cv::Scalar(255));
        tiny.copyTo(largeWindow(cv::Rect(967, 517, tiny.cols, tiny.rows)));
        const auto wholeWindow = qr::analyze(largeWindow);
        expectText(wholeWindow, "https://example.org/qreader?large-window=1");
        expectText(qr::analyze(tiny), "https://example.org/qreader?large-window=1");
        require(wholeWindow.codes.size() == 1, "Overlapping tiles duplicated the same location");
        require(wholeWindow.analyzed.width <= 1920 && wholeWindow.analyzed.height == 1080,
                "Large-window overview limit");
        const auto center =
            (wholeWindow.codes[0].corners[0] + wholeWindow.codes[0].corners[2]) * 0.5f;
        const double expectedX =
            (967 + tiny.cols / 2.0) * wholeWindow.analyzed.width / largeWindow.cols;
        const double expectedY =
            (517 + tiny.rows / 2.0) * wholeWindow.analyzed.height / largeWindow.rows;
        require(std::abs(center.x - expectedX) < 3 && std::abs(center.y - expectedY) < 3,
                "Tile location was not mapped to overview");
        cv::imwrite("fixtures/large-window.png", largeWindow);
        // Include a second identical symbol and a larger code visible in the
        // overview: merge by position, preserve payload duplicates and big codes.
        tiny.copyTo(largeWindow(cv::Rect(1977, 1200, tiny.cols, tiny.rows)));
        qrImage.copyTo(largeWindow(cv::Rect(30, 40, qrImage.cols, qrImage.rows)));
        const auto largeMany = qr::analyze(largeWindow);
        require(largeMany.codes.size() == 3,
                "Large-window multi-code locations lost or duplicated");
        require(std::count_if(largeMany.codes.begin(), largeMany.codes.end(),
                              [](const auto &code) {
                                  return code.decoded &&
                                         code.text == "https://example.org/qreader?large-window=1";
                              }) == 2,
                "Identical small payloads at distinct tile positions lost");
        expectText(largeMany, payload);
        expectSame(qr::analyze(largeWindow), largeMany);
        std::cout << "PASS large-window small QR, original crop, overview mapping and tile "
                     "deduplication\n";

        // Dense small-code row inside a dark browser-like frame. Broad tiles
        // can miss every symbol even though individual crops decode correctly.
        cv::Mat crowded(1300, 2100, CV_8UC1, cv::Scalar(220));
        cv::rectangle(crowded, {220, 180, 1500, 950}, cv::Scalar(25), cv::FILLED);
        cv::rectangle(crowded, {240, 250, 1460, 850}, cv::Scalar(255), cv::FILLED);
        std::vector<std::string> rowPayloads;
        for (int i = 0; i < 7; ++i) {
            rowPayloads.push_back("https://example.org/dense-row/" + std::to_string(i) +
                                  "?source=qreader-regression");
            const auto small = symbol(rowPayloads.back(), 90);
            small.copyTo(crowded(cv::Rect(500 + i * 112, 550, small.cols, small.rows)));
        }
        const auto crowdedResult = qr::analyze(crowded);
        // OpenCV can also return undecodable candidates within the dark frame.
        // Those remain visible by design; require exactly one decoded result
        // at each real position, without accepting extra decoded content.
        require(std::count_if(crowdedResult.codes.begin(), crowdedResult.codes.end(),
                              [](const auto &code) { return code.decoded; }) ==
                    static_cast<int>(rowPayloads.size()),
                "Dense row decoded locations lost or duplicated");
        for (size_t i = 0; i < rowPayloads.size(); ++i) {
            require(std::count_if(crowdedResult.codes.begin(), crowdedResult.codes.end(),
                                  [&](const auto &code) {
                                      return code.decoded && code.text == rowPayloads[i];
                                  }) == 1,
                    "Dense row content lost or duplicated");
            const auto code =
                std::find_if(crowdedResult.codes.begin(), crowdedResult.codes.end(),
                             [&](const auto &c) { return c.decoded && c.text == rowPayloads[i]; });
            const auto center = (code->corners[0] + code->corners[2]) * 0.5f;
            require(std::abs(center.x - (545 + i * 112) * double(crowdedResult.analyzed.width) /
                                            crowded.cols) < 3 &&
                        std::abs(center.y -
                                 595 * double(crowdedResult.analyzed.height) / crowded.rows) < 3,
                    "Dense row content assigned to wrong position");
        }
        std::cout << "PASS compact proposals for dense small-code row inside dark frame\n";

        cv::Mat alpha;
        cv::cvtColor(qrImage, alpha, cv::COLOR_GRAY2BGRA);
        for (int y = 0; y < alpha.rows; ++y)
            for (int x = 0; x < alpha.cols; ++x)
                if (qrImage.at<unsigned char>(y, x) == 255)
                    alpha.at<cv::Vec4b>(y, x) = {0, 0, 0, 0};
        expectText(qr::analyze(alpha), payload);
        cv::Mat sixteen;
        qrImage.convertTo(sixteen, CV_16U, 257);
        expectText(qr::analyze(sixteen), payload);
        auto damaged = qrImage.clone();
        damaged(cv::Rect(80, 80, 100, 100)).setTo(255);
        const auto failed = qr::analyze(damaged);
        require(!failed.codes.empty() && !failed.codes[0].decoded,
                "Undecodable location must be retained");
        std::cout << "PASS transparent and 16-bit PNG pixels, undecodable location\n";

        const auto dir = std::filesystem::current_path() / "fixtures";
        std::filesystem::create_directories(dir);
        for (const std::string extension : {".bmp", ".jpg", ".png"}) {
            auto path = dir / std::filesystem::u8path("二维码 空格" + extension);
            std::vector<unsigned char> bytes;
            cv::imencode(extension, qrImage, bytes);
            {
                std::ofstream file(path, std::ios::binary);
                file.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
            }
            expectText(qr::analyze(qr::loadImage(path)), payload);
        }
        cv::imwrite((dir / "multi.png").string(), multiple);
        {
            std::ofstream file(dir / "invalid.png");
            file << "not an image";
        }
        for (auto path : {dir / "invalid.png", dir / "missing.png"}) {
            bool threw = false;
            try {
                qr::loadImage(path);
            } catch (const std::exception &) {
                threw = true;
            }
            require(threw, "Invalid image should report error");
        }
        bool threw = false;
        try {
            qr::analyze({});
        } catch (const std::exception &) {
            threw = true;
        }
        require(threw, "Empty input should report error");
        std::cout << "PASS BMP/JPG/PNG, Unicode paths, invalid/missing/empty input\n";

        const auto smallMulti = qr::loadImage(dir / "multi-small-qr.png");
        expectText(qr::analyze(smallMulti(cv::Rect(300, 316, 120, 123))),
                   "https://example.org/test-payload-number-1");
        const auto multiResult = qr::analyze(smallMulti);
        expectText(multiResult, "https://example.org/test-payload-number-1");
        expectText(multiResult, "https://example.org/test-payload-number-2");
        // Neither localization may acquire the decoded text from its neighbor.
        for (const auto &code : multiResult.codes) {
            const auto center = (code.corners[0] + code.corners[2]) * 0.5f;
            if (center.x > 300 && center.x < 450)
                require(code.text == "https://example.org/test-payload-number-1",
                        "Wrong neighbor payload assigned");
            if (center.x > 550)
                require(code.text == "https://example.org/test-payload-number-2",
                        "Wrong neighbor payload assigned");
        }
        std::cout << "PASS small QR in isolation and in a multiple-code image\n";

        const auto seedSensitive = qr::loadImage(dir / "detection-seed-sensitive.png");
        const auto seedResult = qr::analyze(seedSensitive);
        for (int run = 0; run < 16; ++run) {
            cv::setRNGSeed(run * 7421 + 11);
            const auto saved = cv::theRNG().state;
            expectSame(qr::analyze(seedSensitive), seedResult);
            expectSame(qr::analyze(smallMulti), multiResult);
            require(cv::theRNG().state == saved, "Analysis changed caller random state");
            qr::Analysis threaded;
            std::exception_ptr failure;
            std::thread worker([&] {
                try {
                    cv::setRNGSeed(run * 37 + 3);
                    threaded = qr::analyze(smallMulti);
                } catch (...) {
                    failure = std::current_exception();
                }
            });
            worker.join();
            if (failure)
                std::rethrow_exception(failure);
            expectSame(threaded, multiResult);
        }
        require(cv::norm(smallMulti, qr::loadImage(dir / "multi-small-qr.png"), cv::NORM_INF) == 0,
                "Analysis modified source pixels");
        std::cout << "PASS repeatability across random states, earlier analyses and fresh worker "
                     "threads\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
