#include "geometry.h"
#include <algorithm>
#include <cmath>

namespace qr
{
Size analysisSize(Size input)
{
    if (input.width <= 0 || input.height <= 0)
        return {};
    const double scale = std::min({1.0, 1920.0 / input.width, 1080.0 / input.height});
    return {std::max(1, static_cast<int>(std::floor(input.width * scale))),
            std::max(1, static_cast<int>(std::floor(input.height * scale)))};
}
Rect fitInside(Size image, Rect bounds)
{
    if (image.width <= 0 || image.height <= 0 || bounds.empty())
        return {};
    const double scale =
        std::min(double(bounds.width()) / image.width, double(bounds.height()) / image.height);
    const int w = std::max(1, static_cast<int>(std::floor(image.width * scale)));
    const int h = std::max(1, static_cast<int>(std::floor(image.height * scale)));
    const int x = bounds.left + (bounds.width() - w) / 2;
    const int y = bounds.top + (bounds.height() - h) / 2;
    return {x, y, x + w, y + h};
}
Rect dragRect(int x1, int y1, int x2, int y2, Rect bounds)
{
    return {std::clamp(std::min(x1, x2), bounds.left, bounds.right),
            std::clamp(std::min(y1, y2), bounds.top, bounds.bottom),
            std::clamp(std::max(x1, x2), bounds.left, bounds.right),
            std::clamp(std::max(y1, y2), bounds.top, bounds.bottom)};
}
} // namespace qr
