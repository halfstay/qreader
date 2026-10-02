#pragma once
#include "geometry.h"
#include <opencv2/core.hpp>
#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace qr
{
struct Code {
    std::array<cv::Point2f, 4> corners{}; // Coordinates in the resized analysis image.
    bool decoded = false;
    std::string text; // UTF-8; identical payloads at distinct locations stay distinct.
};
struct Analysis {
    Size original;
    Size analyzed;
    std::vector<Code> codes;
};
cv::Mat loadImage(const std::filesystem::path &path);
cv::Mat prepareImage(const cv::Mat &image);
Analysis analyze(const cv::Mat &image);
} // namespace qr
