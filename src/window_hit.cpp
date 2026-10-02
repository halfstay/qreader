#include "window_hit.h"
#include <dwmapi.h>

namespace qr
{
namespace
{
bool inRegion(HWND window, POINT point)
{
    RECT outer{};
    if (!GetWindowRect(window, &outer))
        return false;
    HRGN region = CreateRectRgn(0, 0, 0, 0);
    if (!region)
        return true;
    const int type = GetWindowRgn(window, region);
    const bool inside =
        type == ERROR || PtInRegion(region, point.x - outer.left, point.y - outer.top);
    DeleteObject(region);
    return inside;
}

WindowHit deepestChild(WindowHit parent, POINT point, unsigned depth = 0)
{
    if (depth >= 128)
        return parent;
    RECT client{};
    if (!GetClientRect(parent.window, &client))
        return parent;
    // Mapping the rectangle as a pair also handles mirrored (RTL) windows.
    MapWindowPoints(parent.window, nullptr, reinterpret_cast<POINT *>(&client), 2);
    RECT visible{};
    if (!IntersectRect(&visible, &client, &parent.bounds) || !PtInRect(&visible, point))
        return parent; // Keep the window's own border/title bar selectable.

    // Direct children in Z order, not EnumChildWindows' flattened descendants.
    // No synchronous messages to foreign windows: hung applications remain capturable.
    for (HWND child = GetTopWindow(parent.window); child; child = GetWindow(child, GW_HWNDNEXT)) {
        if (!IsWindowVisible(child) || !(GetWindowLongPtrW(child, GWL_STYLE) & WS_CHILD))
            continue;
        if (GetWindowLongPtrW(child, GWL_EXSTYLE) & WS_EX_TRANSPARENT)
            continue;
        RECT bounds{}, clipped{};
        if (!GetWindowRect(child, &bounds) || !IntersectRect(&clipped, &bounds, &visible))
            continue;
        if (!PtInRect(&clipped, point) || !inRegion(child, point))
            continue;
        return deepestChild({child, clipped}, point, depth + 1);
    }
    return parent;
}

struct Search {
    POINT point;
    HWND overlay;
    HWND owner;
    WindowHit hit;
};
BOOL CALLBACK topWindow(HWND hwnd, LPARAM parameter)
{
    auto &search = *reinterpret_cast<Search *>(parameter);
    if (hwnd == search.overlay || hwnd == search.owner || !IsWindowVisible(hwnd) || IsIconic(hwnd))
        return TRUE;
    const auto style = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    if ((style & WS_EX_TRANSPARENT) && (style & WS_EX_LAYERED))
        return TRUE;
    DWORD cloaked = 0;
    // Attribute 14 is unavailable on Win7; a failed query leaves cloaked=0.
    DwmGetWindowAttribute(hwnd, 14, &cloaked, sizeof(cloaked));
    if (cloaked)
        return TRUE;
    RECT bounds{};
    if (FAILED(DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &bounds, sizeof(bounds))))
        if (!GetWindowRect(hwnd, &bounds))
            return TRUE;
    if (!PtInRect(&bounds, search.point) || !inRegion(hwnd, search.point))
        return TRUE;
    search.hit = deepestChild({hwnd, bounds}, search.point);
    return FALSE;
}
} // namespace

WindowHit windowAtPoint(POINT point, HWND overlay, HWND owner)
{
    Search search{point, overlay, owner, {}};
    EnumWindows(topWindow, reinterpret_cast<LPARAM>(&search));
    return search.hit;
}
} // namespace qr
