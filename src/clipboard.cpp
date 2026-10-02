#include "error.h"
#include "clipboard.h"
#include "analyzer.h"
#include <shellapi.h>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <cwctype>
#include <limits>
#include <stdexcept>

namespace qr
{
namespace
{
constexpr size_t maxBytes = 256u * 1024u * 1024u;
constexpr DWORD profileEmbedded = 0x4d424544; // PROFILE_EMBEDDED ('MBED').
constexpr DWORD profileLinked = 0x4c494e4b;   // PROFILE_LINKED ('LINK').
struct ClipboardLock {
    explicit ClipboardLock(HWND owner)
    {
        if (!OpenClipboard(owner))
            throw qr::Error("error.clipboard_busy");
    }
    ~ClipboardLock()
    {
        CloseClipboard();
    }
};
std::vector<unsigned char> copyMemory(HGLOBAL memory)
{
    const auto size = GlobalSize(memory);
    if (!size || size > maxBytes)
        throw qr::Error("error.clipboard_invalid");
    const auto *data = static_cast<const unsigned char *>(GlobalLock(memory));
    if (!data)
        throw qr::Error("error.clipboard_invalid");
    try {
        std::vector<unsigned char> result(data, data + size);
        GlobalUnlock(memory);
        return result;
    } catch (...) {
        GlobalUnlock(memory);
        throw;
    }
}
cv::Mat copyBitmap(HBITMAP bitmap)
{
    BITMAP details{};
    if (!bitmap || !GetObjectW(bitmap, sizeof(details), &details) || details.bmWidth <= 0 ||
        details.bmHeight <= 0 || uint64_t(details.bmWidth) * details.bmHeight > maxBytes / 4)
        throw qr::Error("error.clipboard_invalid");
    cv::Mat pixels(details.bmHeight, details.bmWidth, CV_8UC4);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = details.bmWidth;
    info.bmiHeader.biHeight = -details.bmHeight;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    HDC dc = GetDC(nullptr);
    const int rows =
        dc ? GetDIBits(dc, bitmap, 0, details.bmHeight, pixels.data, &info, DIB_RGB_COLORS) : 0;
    if (dc)
        ReleaseDC(nullptr, dc);
    if (rows != details.bmHeight)
        throw qr::Error("error.clipboard_invalid");
    // CF_BITMAP has no alpha semantics; GetDIBits' reserved bytes are not alpha.
    for (int y = 0; y < pixels.rows; ++y)
        for (int x = 0; x < pixels.cols; ++x)
            pixels.at<cv::Vec4b>(y, x)[3] = 255;
    return pixels;
}
template <class T> T read(const std::vector<unsigned char> &bytes, size_t offset)
{
    if (offset > bytes.size() || sizeof(T) > bytes.size() - offset)
        throw qr::Error("error.clipboard_invalid");
    T value{};
    std::memcpy(&value, bytes.data() + offset, sizeof(value));
    return value;
}
bool validMask(uint32_t mask, unsigned bits)
{
    if (!mask || (bits < 32 && (mask >> bits)))
        return false;
    while (!(mask & 1))
        mask >>= 1;
    return (mask & (mask + 1)) == 0; // A contiguous run of bits.
}
unsigned char channel(uint32_t pixel, uint32_t mask)
{
    while (!(mask & 1)) {
        mask >>= 1;
        pixel >>= 1;
    }
    return static_cast<unsigned char>((uint64_t(pixel & mask) * 255 + mask / 2) / mask);
}
} // namespace

void copyScreenshot(HWND owner, const cv::Mat &bgra)
{
    if (bgra.empty() || bgra.type() != CV_8UC4)
        throw qr::Error("error.clipboard_invalid");
    const size_t stride = (size_t(bgra.cols) * 3 + 3) & ~size_t(3);
    if (stride > (maxBytes - sizeof(BITMAPINFOHEADER)) / size_t(bgra.rows))
        throw qr::Error("error.clipboard_write");
    const size_t pixels = stride * bgra.rows;
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(BITMAPINFOHEADER) + pixels);
    if (!memory)
        throw qr::Error("error.memory");
    auto *bytes = static_cast<unsigned char *>(GlobalLock(memory));
    if (!bytes) {
        GlobalFree(memory);
        throw qr::Error("error.clipboard_write");
    }
    BITMAPINFOHEADER header{};
    header.biSize = sizeof(header);
    header.biWidth = bgra.cols;
    header.biHeight = bgra.rows;
    header.biPlanes = 1;
    header.biBitCount = 24;
    header.biSizeImage = static_cast<DWORD>(pixels);
    std::memcpy(bytes, &header, sizeof(header));
    // Bottom-up 24-bit CF_DIB is supported by Windows 7 and common image editors.
    // Copy the unscaled capture, respecting source stride and DIB row padding.
    for (int y = 0; y < bgra.rows; ++y) {
        const auto *source = bgra.ptr<cv::Vec4b>(y);
        auto *target = bytes + sizeof(header) + size_t(bgra.rows - 1 - y) * stride;
        for (int x = 0; x < bgra.cols; ++x)
            for (int c = 0; c < 3; ++c)
                target[size_t(x) * 3 + c] = source[x][c];
    }
    GlobalUnlock(memory);
    try {
        ClipboardLock lock(owner);
        if (!EmptyClipboard() || !SetClipboardData(CF_DIB, memory))
            throw qr::Error("error.clipboard_write");
    } catch (...) {
        GlobalFree(memory);
        throw;
    }
    // SetClipboardData transfers ownership to Windows.
}

ClipboardImage readClipboardImage(HWND owner)
{
    ClipboardLock lock(owner);
    ClipboardImage input;
    // Prefer lossless PNG with explicit alpha (browsers and image editors).
    const UINT png = RegisterClipboardFormatW(L"PNG");
    for (const UINT format : {png, UINT(CF_DIBV5), UINT(CF_DIB)}) {
        if (format && IsClipboardFormatAvailable(format)) {
            HANDLE memory = GetClipboardData(format);
            if (!memory)
                continue;
            input.format =
                format == png ? ClipboardImage::Format::Png : ClipboardImage::Format::Dib;
            input.bytes = copyMemory(memory);
            if (format == CF_DIBV5 && IsClipboardFormatAvailable(CF_DIB)) {
                // In particular, Wine can synthesize a V5 indexed image without
                // its palette. Preserve the original DIB for a worker-side retry.
                if (HANDLE fallback = GetClipboardData(CF_DIB))
                    input.fallbackDib = copyMemory(fallback);
            }
            return input;
        }
    }
    if (IsClipboardFormatAvailable(CF_BITMAP)) {
        input.format = ClipboardImage::Format::Bitmap;
        input.bitmap = copyBitmap(static_cast<HBITMAP>(GetClipboardData(CF_BITMAP)));
        return input;
    }
    if (IsClipboardFormatAvailable(CF_HDROP)) {
        auto drop = static_cast<HDROP>(GetClipboardData(CF_HDROP));
        if (!drop || DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0) != 1)
            throw qr::Error("error.clipboard_files");
        std::wstring path(DragQueryFileW(drop, 0, nullptr, 0) + 1, L'\0');
        if (!DragQueryFileW(drop, 0, path.data(), static_cast<UINT>(path.size())))
            throw qr::Error("error.clipboard_invalid");
        path.resize(wcslen(path.c_str()));
        input.path = path;
        auto extension = input.path.extension().wstring();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](wchar_t ch) { return std::towlower(ch); });
        if (extension != L".bmp" && extension != L".jpg" && extension != L".jpeg" &&
            extension != L".png")
            throw qr::Error("error.clipboard_format");
        input.format = ClipboardImage::Format::File;
        return input;
    }
    throw qr::Error("error.clipboard_empty");
}

cv::Mat decodeDib(const std::vector<unsigned char> &bytes)
{
    const auto h = read<BITMAPINFOHEADER>(bytes, 0);
    if ((h.biSize != 40 && h.biSize != 52 && h.biSize != 56 && h.biSize != 108 &&
         h.biSize != 124) ||
        h.biSize > bytes.size() || h.biPlanes != 1 || h.biWidth <= 0 || h.biHeight == 0 ||
        h.biHeight == std::numeric_limits<LONG>::min())
        throw qr::Error("error.clipboard_invalid");
    const unsigned width = h.biWidth, height = h.biHeight < 0 ? -h.biHeight : h.biHeight;
    const unsigned bits = h.biBitCount;
    const bool bitfields =
        h.biCompression == BI_BITFIELDS || h.biCompression == 6; // BI_ALPHABITFIELDS
    if ((bits != 1 && bits != 4 && bits != 8 && bits != 16 && bits != 24 && bits != 32) ||
        (h.biCompression != BI_RGB && !bitfields) || (bitfields && bits != 16 && bits != 32) ||
        uint64_t(width) * height > maxBytes / 4)
        throw qr::Error("error.clipboard_format");
    size_t offset = h.biSize;
    std::array<uint32_t, 4> masks{0x7c00, 0x03e0, 0x001f, 0};
    if (bits == 32)
        masks = {0x00ff0000, 0x0000ff00, 0x000000ff, 0};
    if (bitfields) {
        for (int i = 0; i < 3; ++i)
            masks[i] = read<uint32_t>(bytes, 40 + 4 * i);
        if (h.biSize >= 56 || h.biCompression == 6)
            masks[3] = read<uint32_t>(bytes, 52);
        if (h.biSize == 40)
            offset += h.biCompression == 6 ? 16 : 12;
        if (h.biCompression == 6 && !masks[3])
            throw qr::Error("error.clipboard_invalid");
        uint32_t used = 0;
        for (int i = 0; i < 4; ++i) {
            if (i == 3 && masks[i] == 0)
                continue;
            if (!validMask(masks[i], bits) || (used & masks[i]))
                throw qr::Error("error.clipboard_invalid");
            used |= masks[i];
        }
    }
    const unsigned colors = h.biClrUsed ? h.biClrUsed : (bits <= 8 ? 1u << bits : 0);
    if (colors > 256 || (bits <= 8 && colors > (1u << bits)))
        throw qr::Error("error.clipboard_invalid");
    std::vector<RGBQUAD> palette;
    for (unsigned i = 0; i < colors; ++i)
        palette.push_back(read<RGBQUAD>(bytes, offset + i * sizeof(RGBQUAD)));
    offset += colors * sizeof(RGBQUAD);
    const uint64_t stride = ((uint64_t(width) * bits + 31) / 32) * 4;
    if (offset > bytes.size() || stride * height > bytes.size() - offset)
        throw qr::Error("error.clipboard_invalid");
    if (h.biSize == sizeof(BITMAPV5HEADER)) {
        const auto v5 = read<BITMAPV5HEADER>(bytes, 0);
        if ((v5.bV5CSType == profileEmbedded || v5.bV5CSType == profileLinked) &&
            v5.bV5ProfileSize) {
            // Packed clipboard DIBs place profiles AFTER the bitmap bits.
            // We never open linked profiles; recognition operates on stored RGB values.
            if (v5.bV5ProfileData < offset + stride * height || v5.bV5ProfileData > bytes.size() ||
                v5.bV5ProfileSize > bytes.size() - v5.bV5ProfileData)
                throw qr::Error("error.clipboard_invalid");
        }
    }
    cv::Mat pixels(height, width, CV_8UC4);
    for (unsigned y = 0; y < height; ++y) {
        const auto *row = bytes.data() + offset + stride * (h.biHeight < 0 ? y : height - 1 - y);
        auto *dst = pixels.ptr<cv::Vec4b>(y);
        for (unsigned x = 0; x < width; ++x) {
            dst[x][3] = 255;
            if (bits <= 8) {
                const unsigned index =
                    (row[x * bits / 8] >> (8 - bits - x * bits % 8)) & ((1u << bits) - 1);
                if (index >= palette.size())
                    throw qr::Error("error.clipboard_invalid");
                dst[x][0] = palette[index].rgbBlue;
                dst[x][1] = palette[index].rgbGreen;
                dst[x][2] = palette[index].rgbRed;
            } else if (bits == 24) {
                for (int c = 0; c < 3; ++c)
                    dst[x][c] = row[x * 3 + c];
            } else {
                uint32_t pixel = 0;
                std::memcpy(&pixel, row + size_t(x) * bits / 8, bits / 8);
                for (int c = 0; c < 3; ++c)
                    dst[x][c] = channel(pixel, masks[2 - c]);
                if (masks[3])
                    dst[x][3] = channel(pixel, masks[3]);
            }
        }
    }
    return pixels;
}

cv::Mat decodeClipboardImage(const ClipboardImage &input)
{
    switch (input.format) {
    case ClipboardImage::Format::File:
        return loadImage(input.path);
    case ClipboardImage::Format::Bitmap:
        return input.bitmap;
    case ClipboardImage::Format::Dib:
        try {
            return decodeDib(input.bytes);
        } catch (const std::runtime_error &) {
            if (input.fallbackDib.empty())
                throw;
            return decodeDib(input.fallbackDib);
        }
    case ClipboardImage::Format::Png: {
        if (input.bytes.empty())
            throw qr::Error("error.clipboard_invalid");
        auto image = cv::imdecode(input.bytes, cv::IMREAD_UNCHANGED);
        if (image.empty())
            throw qr::Error("error.clipboard_invalid");
        return image;
    }
    }
    throw qr::Error("error.clipboard_format");
}
} // namespace qr
