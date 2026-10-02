#include "error.h"
#include "capture.h"
#include "window_hit.h"
#include "win_util.h"
#include "i18n.h"
#include <dwmapi.h>
#include <windowsx.h>
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace qr
{
namespace
{
constexpr wchar_t captureClass[] = L"QReader.Capture";
cv::Mat snapshot(Rect bounds)
{
    HDC screen = GetDC(nullptr);
    if (!screen)
        throw qr::Error("error.capture");
    HDC memory = CreateCompatibleDC(screen);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = bounds.width();
    info.bmiHeader.biHeight = -bounds.height();
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void *bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(screen, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!memory || !bitmap) {
        if (bitmap)
            DeleteObject(bitmap);
        if (memory)
            DeleteDC(memory);
        ReleaseDC(nullptr, screen);
        throw qr::Error("error.capture");
    }
    const auto old = SelectObject(memory, bitmap);
    const bool ok = BitBlt(memory, 0, 0, bounds.width(), bounds.height(), screen, bounds.left,
                           bounds.top, SRCCOPY | CAPTUREBLT);
    GdiFlush();
    cv::Mat result;
    try {
        if (ok)
            result = cv::Mat(bounds.height(), bounds.width(), CV_8UC4, bits).clone();
    } catch (...) {
        SelectObject(memory, old);
        DeleteObject(bitmap);
        DeleteDC(memory);
        ReleaseDC(nullptr, screen);
        throw;
    }
    SelectObject(memory, old);
    DeleteObject(bitmap);
    DeleteDC(memory);
    ReleaseDC(nullptr, screen);
    if (!ok)
        throw qr::Error("error.capture");
    // BI_RGB's fourth byte is reserved; it is not meaningful transparency.
    for (int y = 0; y < result.rows; ++y) {
        auto *row = result.ptr<cv::Vec4b>(y);
        for (int x = 0; x < result.cols; ++x)
            row[x][3] = 255;
    }
    return result;
}
} // namespace

Capture::~Capture()
{
    if (window_)
        DestroyWindow(window_);
}
void Capture::begin(HWND owner, std::function<void(cv::Mat)> complete)
{
    if (active())
        return;
    owner_ = owner;
    desktopBounds_ = {GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN), 0,
                      0};
    desktopBounds_.right = desktopBounds_.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    desktopBounds_.bottom = desktopBounds_.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (desktopBounds_.empty())
        throw qr::Error("error.capture");
    desktop_ = snapshot(desktopBounds_);
    complete_ = std::move(complete);
    WNDCLASSEXW cls{};
    cls.cbSize = sizeof(cls);
    cls.hInstance = instance_;
    cls.lpszClassName = captureClass;
    cls.lpfnWndProc = procedure;
    cls.hCursor = LoadCursorW(nullptr, IDC_CROSS);
    if (!RegisterClassExW(&cls) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        throw qr::Error("error.capture");
    pressed_ = dragging_ = false;
    window_ =
        CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, captureClass, i18n::text("capture"),
                        WS_POPUP, desktopBounds_.left, desktopBounds_.top, desktopBounds_.width(),
                        desktopBounds_.height(), nullptr, nullptr, instance_, this);
    if (!window_)
        throw qr::Error("error.capture");
    POINT point{};
    GetCursorPos(&point);
    update(point);
    ShowWindow(window_, SW_SHOW);
    SetForegroundWindow(window_);
    SetFocus(window_);
}
void Capture::cancel()
{
    if (active())
        finish(false);
}
void Capture::update(POINT point)
{
    if (pressed_) {
        dragging_ = dragging_ || std::abs(point.x - anchor_.x) >= GetSystemMetrics(SM_CXDRAG) ||
                    std::abs(point.y - anchor_.y) >= GetSystemMetrics(SM_CYDRAG);
        if (dragging_)
            selection_ = dragRect(anchor_.x, anchor_.y, point.x, point.y, desktopBounds_);
    } else {
        const auto hit = windowAtPoint(point, window_, owner_);
        if (hit.window)
            selection_ = dragRect(hit.bounds.left, hit.bounds.top, hit.bounds.right,
                                  hit.bounds.bottom, desktopBounds_);
        else {
            MONITORINFO monitor{};
            monitor.cbSize = sizeof(monitor);
            if (GetMonitorInfoW(MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST), &monitor))
                selection_ =
                    dragRect(monitor.rcMonitor.left, monitor.rcMonitor.top, monitor.rcMonitor.right,
                             monitor.rcMonitor.bottom, desktopBounds_);
            else
                selection_ = desktopBounds_;
        }
    }
    InvalidateRect(window_, nullptr, FALSE);
}
void Capture::finish(bool accept)
{
    cv::Mat image;
    if (accept && !selection_.empty()) {
        image = desktop_(cv::Rect(selection_.left - desktopBounds_.left,
                                  selection_.top - desktopBounds_.top, selection_.width(),
                                  selection_.height()))
                    .clone();
    }
    HWND closing = std::exchange(window_, nullptr);
    // Clear window_ before releasing mouse capture (WM_CAPTURECHANGED is synchronous).
    if (GetCapture() == closing)
        ReleaseCapture();
    DestroyWindow(closing);
    desktop_.release();
    auto complete = std::move(complete_);
    if (complete)
        complete(std::move(image));
}
void Capture::paint()
{
    PAINTSTRUCT ps{};
    HDC dc = BeginPaint(window_, &ps);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = desktop_.cols;
    info.bmiHeader.biHeight = -desktop_.rows;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    SetDIBitsToDevice(dc, 0, 0, desktop_.cols, desktop_.rows, 0, 0, 0, desktop_.rows, desktop_.data,
                      &info, DIB_RGB_COLORS);
    RECT rect{selection_.left - desktopBounds_.left, selection_.top - desktopBounds_.top,
              selection_.right - desktopBounds_.left, selection_.bottom - desktopBounds_.top};
    HBRUSH brush = CreateSolidBrush(RGB(0, 255, 0));
    for (int i = 0; i < 3 && rect.right > rect.left && rect.bottom > rect.top; ++i) {
        FrameRect(dc, &rect, brush);
        InflateRect(&rect, -1, -1);
    }
    DeleteObject(brush);
    EndPaint(window_, &ps);
}
LRESULT CALLBACK Capture::procedure(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    auto *self = reinterpret_cast<Capture *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        self = static_cast<Capture *>(reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams);
        self->window_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    if (!self || !self->window_)
        return DefWindowProcW(hwnd, message, wp, lp);
    try {
        return self->dispatch(message, wp, lp);
    } catch (...) {
        self->finish(false);
        showError(self->owner_, i18n::text("error.capture"));
        return 0;
    }
}
LRESULT Capture::dispatch(UINT message, WPARAM wp, LPARAM lp)
{
    switch (message) {
    case WM_PAINT:
        paint();
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_MOUSEMOVE: {
        POINT point{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        ClientToScreen(window_, &point);
        update(point);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        POINT point{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        ClientToScreen(window_, &point);
        update(point);
        anchor_ = point;
        pressed_ = true;
        dragging_ = false;
        SetCapture(window_);
        return 0;
    }
    case WM_LBUTTONUP: {
        if (!pressed_)
            return 0;
        POINT point{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        ClientToScreen(window_, &point);
        update(point);
        // A line has no area; leave selection mode active rather than silently capturing a window.
        if (dragging_ && selection_.empty()) {
            pressed_ = dragging_ = false;
            ReleaseCapture();
            update(point);
            return 0;
        }
        finish(true);
        return 0;
    }
    case WM_RBUTTONDOWN:
    case WM_CLOSE:
    case WM_DISPLAYCHANGE:
        finish(false);
        return 0;
    case WM_KEYDOWN:
        if (wp == VK_ESCAPE) {
            finish(false);
            return 0;
        }
        break;
    case WM_CAPTURECHANGED:
        if (pressed_) {
            finish(false);
            return 0;
        }
        break;
    case WM_ACTIVATE:
        if (LOWORD(wp) == WA_INACTIVE) {
            finish(false);
            return 0;
        }
        break;
    }
    return DefWindowProcW(window_, message, wp, lp);
}
} // namespace qr
