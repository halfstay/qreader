#pragma once

namespace qr
{
struct Size {
    int width = 0;
    int height = 0;
};
struct Rect {
    int left = 0, top = 0, right = 0, bottom = 0;
    int width() const
    {
        return right - left;
    }
    int height() const
    {
        return bottom - top;
    }
    bool empty() const
    {
        return width() <= 0 || height() <= 0;
    }
};
Size analysisSize(Size input);
Rect fitInside(Size image, Rect bounds);
Rect dragRect(int x1, int y1, int x2, int y2, Rect bounds);
} // namespace qr
