#pragma once
#include <windows.h>
#include <opencv2/core.hpp>
#include <filesystem>
#include <vector>

namespace qr
{
// Own the snapshot so the clipboard can change during background analysis.
struct ClipboardImage {
    enum class Format { Png, Dib, Bitmap, File } format = Format::Png;
    std::vector<unsigned char> bytes;
    std::vector<unsigned char> fallbackDib; // Some clipboard providers synthesize an invalid DIBV5.
    cv::Mat bitmap;
    std::filesystem::path path;
};
ClipboardImage readClipboardImage(HWND owner);
void copyScreenshot(HWND owner, const cv::Mat &bgra);
cv::Mat decodeClipboardImage(const ClipboardImage &input);
cv::Mat decodeDib(const std::vector<unsigned char> &bytes);
} // namespace qr
