#include "clipboard.h"
#include "analyzer.h"
#include "window_hit.h"
#include <opencv2/imgcodecs.hpp>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>

static void require(bool ok, const char *text)
{
    if (!ok)
        throw std::runtime_error(text);
}
template <class T> static void put(std::vector<unsigned char> &data, size_t offset, const T &value)
{
    require(offset + sizeof(T) <= data.size(), "Invalid fixture write");
    std::memcpy(data.data() + offset, &value, sizeof(T));
}
static void expectBad(const std::vector<unsigned char> &bytes)
{
    bool threw = false;
    try {
        qr::decodeDib(bytes);
    } catch (const std::exception &) {
        threw = true;
    }
    require(threw, "Malformed DIB was accepted");
}
static void dibTests()
{
    BITMAPINFOHEADER header{};
    header.biSize = sizeof(header);
    header.biWidth = 3;
    header.biHeight = 2;
    header.biPlanes = 1;
    header.biBitCount = 24;
    std::vector<unsigned char> bytes(sizeof(header) + 24, 0);
    put(bytes, 0, header);
    bytes[40] = 255; // Blue on bottom row, red on top row; each row has 3 padding bytes.
    bytes[54] = 255;
    auto image = qr::decodeDib(bytes);
    require(image.at<cv::Vec4b>(0, 0) == cv::Vec4b(0, 0, 255, 255), "Bottom-up DIB row order");
    require(image.at<cv::Vec4b>(1, 0) == cv::Vec4b(255, 0, 0, 255), "DIB row alignment");
    header.biHeight = -2;
    put(bytes, 0, header);
    image = qr::decodeDib(bytes);
    require(image.at<cv::Vec4b>(0, 0) == cv::Vec4b(255, 0, 0, 255), "Top-down DIB row order");
    header.biBitCount = 32;
    put(bytes, 0, header);
    require(qr::decodeDib(bytes).at<cv::Vec4b>(0, 0)[3] == 255,
            "BI_RGB reserved byte mistaken for alpha");

    BITMAPV5HEADER v5{};
    v5.bV5Size = sizeof(v5);
    v5.bV5Width = 1;
    v5.bV5Height = -1;
    v5.bV5Planes = 1;
    v5.bV5BitCount = 32;
    v5.bV5Compression = BI_BITFIELDS;
    v5.bV5RedMask = 0xff0000;
    v5.bV5GreenMask = 0xff00;
    v5.bV5BlueMask = 0xff;
    v5.bV5AlphaMask = 0xff000000;
    bytes.assign(sizeof(v5) + 8, 0);
    put(bytes, 0, v5);
    put<uint32_t>(bytes, sizeof(v5), 0x8000ff00);
    require(qr::decodeDib(bytes).at<cv::Vec4b>(0, 0) == cv::Vec4b(0, 255, 0, 128),
            "V5 alpha bitfields");
    v5.bV5CSType = 0x4d424544;
    v5.bV5ProfileData = sizeof(v5) + 4;
    v5.bV5ProfileSize = 4;
    put(bytes, 0, v5);
    require(qr::decodeDib(bytes).at<cv::Vec4b>(0, 0)[1] == 255,
            "Packed DIB profile changed pixel offset");
    v5.bV5ProfileData = sizeof(v5);
    put(bytes, 0, v5);
    expectBad(bytes);

    header.biWidth = 1;
    header.biHeight = 1;
    header.biBitCount = 16;
    header.biCompression = BI_BITFIELDS;
    bytes.assign(56, 0);
    put(bytes, 0, header);
    put<uint32_t>(bytes, 40, 0xf800);
    put<uint32_t>(bytes, 44, 0x07e0);
    put<uint32_t>(bytes, 48, 0x001f);
    put<uint16_t>(bytes, 52, 0x07e0);
    require(qr::decodeDib(bytes).at<cv::Vec4b>(0, 0) == cv::Vec4b(0, 255, 0, 255),
            "16-bit 565 DIB");
    put<uint32_t>(bytes, 44, 0xf800);
    expectBad(bytes);

    for (const WORD bits : {1, 4, 8}) {
        header.biBitCount = bits;
        header.biCompression = BI_RGB;
        header.biClrUsed = 2;
        header.biWidth = 2;
        bytes.assign(52, 0);
        put(bytes, 0, header);
        put<RGBQUAD>(bytes, 44, {255, 255, 255, 0});
        bytes[48] = bits == 1 ? 0x40 : bits == 4 ? 0x01 : 0;
        if (bits == 8)
            bytes[49] = 1;
        image = qr::decodeDib(bytes);
        require(image.at<cv::Vec4b>(0, 0)[0] == 0 && image.at<cv::Vec4b>(0, 1)[0] == 255,
                "Indexed DIB palette");
    }
    bytes.pop_back();
    expectBad(bytes);
    expectBad({});
    bytes.resize(52);
    header.biWidth = LONG_MAX;
    put(bytes, 0, header);
    expectBad(bytes);
    std::cout
        << "PASS DIB orientations, stride, alpha, palette, bitfields, profiles, malformed input\n";
}
static void setMemory(HWND owner, UINT format, const std::vector<unsigned char> &bytes)
{
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes.size());
    require(memory != nullptr, "Clipboard test allocation");
    void *data = GlobalLock(memory);
    require(data != nullptr, "Clipboard test lock");
    std::memcpy(data, bytes.data(), bytes.size());
    GlobalUnlock(memory);
    require(OpenClipboard(owner), "Clipboard test open");
    EmptyClipboard();
    const bool ok = SetClipboardData(format, memory) != nullptr;
    CloseClipboard();
    if (!ok)
        GlobalFree(memory);
    require(ok, "Clipboard test write");
}
static HWND child(HWND parent, int x, int y, int width, int height, DWORD ex = 0)
{
    HWND result = CreateWindowExW(ex, L"STATIC", L"", WS_CHILD | WS_VISIBLE, x, y, width, height,
                                  parent, nullptr, nullptr, nullptr);
    require(result != nullptr, "Cannot create hit-test child");
    return result;
}
int main()
{
    HWND parent = nullptr;
    try {
        dibTests();
        parent =
            CreateWindowExW(WS_EX_TOPMOST, L"STATIC", L"QReader hit tests", WS_POPUP | WS_VISIBLE,
                            40, 40, 400, 320, nullptr, nullptr, nullptr, nullptr);
        require(parent != nullptr, "Cannot create hit-test parent");
        const auto panel = child(parent, 20, 20, 200, 180);
        const auto nested = child(panel, 10, 10, 60, 50);
        auto hit = qr::windowAtPoint({75, 75}, nullptr, nullptr);
        require(hit.window == nested && hit.bounds.left == 70 && hit.bounds.right == 130,
                "Nested child not selected");
        const auto cover = child(panel, 5, 5, 100, 100);
        SetWindowPos(cover, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        require(qr::windowAtPoint({75, 75}, nullptr, nullptr).window == cover,
                "Topmost sibling not selected");
        ShowWindow(cover, SW_HIDE);
        require(qr::windowAtPoint({75, 75}, nullptr, nullptr).window == nested,
                "Hidden child blocks hit");
        ShowWindow(cover, SW_SHOW);
        SetWindowLongPtrW(cover, GWL_EXSTYLE, WS_EX_TRANSPARENT);
        require(qr::windowAtPoint({75, 75}, nullptr, nullptr).window == nested,
                "Transparent child blocks hit");
        EnableWindow(nested, FALSE);
        require(qr::windowAtPoint({75, 75}, nullptr, nullptr).window == nested,
                "Visible disabled child lost");
        require(qr::windowAtPoint({45, 45}, nullptr, nullptr).window == parent,
                "Parent background lost");
        const auto clipped = child(panel, 180, 150, 100, 100);
        hit = qr::windowAtPoint({245, 215}, nullptr, nullptr);
        require(hit.window == clipped && hit.bounds.right == 260 && hit.bounds.bottom == 240,
                "Child exceeds ancestor bounds");
        require(qr::windowAtPoint({275, 250}, nullptr, nullptr).window == parent,
                "Child outside parent client selected");
        std::cout << "PASS nested children, sibling Z order, hidden/transparent/disabled controls, "
                     "ancestor clipping\n";

        cv::Mat png(2, 3, CV_8UC3, cv::Scalar(0, 255, 0));
        std::vector<unsigned char> encoded;
        cv::imencode(".png", png, encoded);
        setMemory(parent, RegisterClipboardFormatW(L"PNG"), encoded);
        const auto input = qr::readClipboardImage(parent);
        require(OpenClipboard(parent), "Clipboard reopen");
        EmptyClipboard();
        CloseClipboard();
        require(qr::decodeClipboardImage(input).at<cv::Vec3b>(0, 0) == cv::Vec3b(0, 255, 0),
                "Clipboard snapshot invalidated");
        bool threw = false;
        try {
            qr::readClipboardImage(parent);
        } catch (const std::exception &) {
            threw = true;
        }
        require(threw, "Empty clipboard must fail");
        std::cout << "PASS PNG clipboard snapshot ownership and empty clipboard error\n";

        BITMAPV5HEADER v5{};
        v5.bV5Size = sizeof(v5);
        v5.bV5Width = 1;
        v5.bV5Height = -1;
        v5.bV5Planes = 1;
        v5.bV5BitCount = 32;
        v5.bV5Compression = BI_BITFIELDS;
        v5.bV5RedMask = 0xff0000;
        v5.bV5GreenMask = 0xff00;
        v5.bV5BlueMask = 0xff;
        v5.bV5AlphaMask = 0xff000000;
        encoded.assign(sizeof(v5) + 4, 0);
        put(encoded, 0, v5);
        put<uint32_t>(encoded, sizeof(v5), 0xff00ff00);
        setMemory(parent, CF_DIBV5, encoded);
        const auto v5Input = qr::readClipboardImage(parent);
        require(qr::decodeClipboardImage(v5Input).at<cv::Vec4b>(0, 0) == cv::Vec4b(0, 255, 0, 255),
                "CF_DIBV5 snapshot");

        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = 2;
        info.bmiHeader.biHeight = -2;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        void *bits = nullptr;
        HDC dc = GetDC(nullptr);
        HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
        ReleaseDC(nullptr, dc);
        require(bitmap != nullptr, "CF_BITMAP fixture allocation");
        std::fill_n(static_cast<uint32_t *>(bits), 4, 0x0000ff00);
        require(OpenClipboard(parent), "CF_BITMAP clipboard open");
        EmptyClipboard();
        const bool published = SetClipboardData(CF_BITMAP, bitmap) != nullptr;
        CloseClipboard();
        if (!published)
            DeleteObject(bitmap);
        require(published, "CF_BITMAP clipboard publish");
        const auto bitmapInput = qr::readClipboardImage(parent);
        require(qr::decodeClipboardImage(bitmapInput).at<cv::Vec4b>(0, 0) ==
                    cv::Vec4b(0, 255, 0, 255),
                "CF_BITMAP reserved alpha");
        require(OpenClipboard(parent), "Clipboard cleanup");
        EmptyClipboard();
        CloseClipboard();
        std::cout << "PASS CF_DIBV5 and CF_BITMAP clipboard import\n";
        cv::imencode(".bmp", qr::loadImage("fixtures/multi.png"), encoded);
        setMemory(parent, CF_DIB, {encoded.begin() + 14, encoded.end()});
        const auto dibInput = qr::readClipboardImage(parent);
        const auto dibImage = qr::decodeClipboardImage(dibInput);
        require(qr::analyze(dibImage).codes.size() == 2, "CF_DIB multi QR decode");
        std::cout << "PASS CF_DIB multi QR decode\n";
        cv::Mat capture(6, 9, CV_8UC4, cv::Scalar(255, 255, 255, 255));
        auto region = capture(cv::Rect(2, 1, 5, 3)); // Non-contiguous input and padded DIB rows.
        region.at<cv::Vec4b>(0, 0) = {0, 0, 255, 255};
        region.at<cv::Vec4b>(2, 4) = {255, 0, 0, 255};
        qr::copyScreenshot(parent, region);
        const auto copied = qr::decodeClipboardImage(qr::readClipboardImage(parent));
        require(copied.size() == region.size() && cv::norm(copied, region, cv::NORM_INF) == 0,
                "Screenshot clipboard changed dimensions, orientation, colors or row stride");
        std::cout << "PASS screenshot clipboard round-trip pixels and padded rows\n";
        DestroyWindow(parent);
        return 0;
    } catch (const std::exception &e) {
        if (parent)
            DestroyWindow(parent);
        std::cerr << "FAIL Windows: " << e.what() << '\n';
        return 1;
    }
}
