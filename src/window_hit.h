#pragma once
#include <windows.h>

namespace qr
{
struct WindowHit {
    HWND window = nullptr;
    RECT bounds{}; // Screen coordinates, clipped to ancestor client areas.
};
WindowHit windowAtPoint(POINT point, HWND overlay, HWND owner);
} // namespace qr
