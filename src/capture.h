#pragma once
#include <windows.h>
#include <opencv2/core.hpp>
#include <functional>
#include "geometry.h"

namespace qr
{
// Owned by the main UI. Completion runs on that same UI thread.
class Capture
{
  public:
    explicit Capture(HINSTANCE instance) : instance_(instance) {}
    ~Capture();
    void begin(HWND owner, std::function<void(cv::Mat)> complete);
    bool active() const
    {
        return window_ != nullptr;
    }
    void cancel();

  private:
    static LRESULT CALLBACK procedure(HWND, UINT, WPARAM, LPARAM);
    LRESULT dispatch(UINT, WPARAM, LPARAM);
    void update(POINT point);
    void finish(bool accept);
    void paint();
    HINSTANCE instance_;
    HWND window_ = nullptr;
    HWND owner_ = nullptr;
    cv::Mat desktop_; // BGRA with opaque alpha; virtual desktop snapshot.
    Rect desktopBounds_;
    Rect selection_;
    POINT anchor_{};
    bool pressed_ = false;
    bool dragging_ = false;
    std::function<void(cv::Mat)> complete_;
};
} // namespace qr
