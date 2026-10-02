#include "analyzer.h"
#include "clipboard.h"
#include "i18n.h"
#include "resource.h"
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <functional>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

static void require(bool ok, const char *message)
{
    if (!ok)
        throw std::runtime_error(message);
}
static bool waitFor(const std::function<bool()> &predicate, int timeout = 15000)
{
    const auto end = GetTickCount64() + timeout;
    do {
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        if (predicate())
            return true;
        Sleep(30);
    } while (GetTickCount64() < end);
    return false;
}
struct Colors {
    int green = 0, red = 0, black = 0;
};
static Colors colors(HWND hwnd)
{
    RECT rect{};
    GetClientRect(hwnd, &rect);
    HDC dc = GetDC(hwnd), mem = CreateCompatibleDC(dc);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = rect.right;
    info.bmiHeader.biHeight = -rect.bottom;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    void *data = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &data, nullptr, 0);
    require(bitmap && mem, "Cannot sample indicator pixels");
    auto old = SelectObject(mem, bitmap);
    BitBlt(mem, 0, 0, rect.right, rect.bottom, dc, 0, 0, SRCCOPY);
    GdiFlush();
    Colors result;
    const auto *pixels = static_cast<unsigned char *>(data);
    for (int i = 0; i < rect.right * rect.bottom; ++i) {
        const auto *p = pixels + 4 * i;
        if (p[0] < 40 && p[1] > 220 && p[2] < 40)
            ++result.green;
        if (p[0] < 80 && p[1] < 80 && p[2] > 220)
            ++result.red;
        if (p[0] == 0 && p[1] == 0 && p[2] == 0)
            ++result.black;
    }
    SelectObject(mem, old);
    DeleteObject(bitmap);
    DeleteDC(mem);
    ReleaseDC(hwnd, dc);
    return result;
}
static void mouse(HWND overlay, UINT message, int x, int y)
{
    POINT point{x, y};
    SetCursorPos(x, y);
    ScreenToClient(overlay, &point);
    SendMessageW(overlay, message, message == WM_LBUTTONUP ? 0 : MK_LBUTTON,
                 MAKELPARAM(point.x, point.y));
}
static void clipboardBytes(HWND owner, UINT format, const std::vector<unsigned char> &bytes)
{
    auto memory = GlobalAlloc(GMEM_MOVEABLE, bytes.size());
    require(memory != nullptr, "Cannot allocate clipboard test data");
    void *data = GlobalLock(memory);
    require(data != nullptr, "Cannot lock clipboard test data");
    std::memcpy(data, bytes.data(), bytes.size());
    GlobalUnlock(memory);
    require(OpenClipboard(owner), "Cannot open image clipboard");
    EmptyClipboard();
    const bool ok = SetClipboardData(format, memory) != nullptr;
    CloseClipboard();
    if (!ok)
        GlobalFree(memory);
    require(ok, "Cannot publish clipboard image");
}
static std::wstring statusText(HWND app, HANDLE process)
{
    wchar_t text[4096]{};
    void *remote =
        VirtualAllocEx(process, nullptr, sizeof(text), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    require(remote != nullptr, "Cannot allocate status read buffer");
    SendMessageW(GetDlgItem(app, ID_STATUS), SB_GETTEXTW, 0, reinterpret_cast<LPARAM>(remote));
    const bool ok = ReadProcessMemory(process, remote, text, sizeof(text), nullptr) != 0;
    VirtualFreeEx(process, remote, 0, MEM_RELEASE);
    require(ok, "Cannot read status text");
    return text;
}
int wmain(int argc, wchar_t **argv)
{
    PROCESS_INFORMATION process{};
    HWND app = nullptr, fixture = nullptr;
    HBITMAP bitmap = nullptr;
    std::string originalLanguage;
    try {
        namespace loc = qr::i18n;
        loc::initialize(GetModuleHandleW(nullptr));
        originalLanguage = loc::preference();
        require(argc == 2, "Pass the QReader executable path");
        const auto multi = std::filesystem::absolute("fixtures/multi.png").wstring();
        std::wstring command = L"\"" + std::wstring(argv[1]) + L"\" \"" + multi + L"\"";
        STARTUPINFOW startup{};
        startup.cb = sizeof(startup);
        require(CreateProcessW(nullptr, command.data(), nullptr, nullptr, FALSE, 0, nullptr,
                               nullptr, &startup, &process),
                "Cannot launch QReader");
        require(waitFor([&] {
                    app = FindWindowW(L"QReader.Main", nullptr);
                    return app != nullptr;
                }),
                "Main window not created");
        wchar_t title[256]{};
        GetWindowTextW(app, title, 256);
        require(std::wstring(title) == loc::text("title"),
                "Default/system window language mismatch");
        const auto chooseLanguage = [&](const std::string &tag) {
            const auto &languages = loc::languages();
            const auto it = std::find_if(languages.begin(), languages.end(),
                                         [&](const auto &value) { return value.tag == tag; });
            require(it != languages.end(), "Unknown test language");
            loc::select(tag, false);
            SendMessageW(app, WM_COMMAND,
                         ID_LANGUAGE_FIRST + static_cast<UINT>(it - languages.begin()), 0);
        };
        chooseLanguage("zh-CN");
        HMODULE resources = LoadLibraryExW(argv[1], nullptr, LOAD_LIBRARY_AS_DATAFILE);
        require(resources && FindResourceW(resources, MAKEINTRESOURCEW(ID_ICON), RT_GROUP_ICON),
                "EXE icon resource missing");
        for (int size : {16, 32, 48, 256}) {
            HICON icon = static_cast<HICON>(
                LoadImageW(resources, MAKEINTRESOURCEW(ID_ICON), IMAGE_ICON, size, size, 0));
            require(icon != nullptr, "Cannot load embedded icon size");
            DestroyIcon(icon);
        }
        FreeLibrary(resources);
        // HICONs are process-local under Wine; validate the resources by loading
        // them in this process. App::run also checks both icon loads at startup.
        HWND list = GetDlgItem(app, ID_RESULTS), indicator = GetDlgItem(app, ID_INDICATOR);
        auto ready = [&] {
            return IsWindowVisible(app) && IsWindowEnabled(GetDlgItem(app, ID_OPEN));
        };
        require(waitFor([&] { return ready() && SendMessageW(list, LVM_GETITEMCOUNT, 0, 0) == 2; }),
                "Two QR results not loaded");
        require(statusText(app, process.hProcess).find(L"分析 ") == std::wstring::npos,
                "Status bar still shows analysis dimensions");
        require(waitFor([&] {
                    auto c = colors(indicator);
                    return c.green > 30 && c.red == 0 && c.black > 1000;
                }),
                "Initial indicator is not black/green");

        LVITEMW item{};
        item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        item.state = item.stateMask;
        void *remote = VirtualAllocEx(process.hProcess, nullptr, sizeof(item),
                                      MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        require(remote &&
                    WriteProcessMemory(process.hProcess, remote, &item, sizeof(item), nullptr),
                "Cannot select remote list row");
        SendMessageW(list, LVM_SETITEMSTATE, 0, reinterpret_cast<LPARAM>(remote));
        VirtualFreeEx(process.hProcess, remote, 0, MEM_RELEASE);
        require(waitFor([&] {
                    auto c = colors(indicator);
                    return c.green > 20 && c.red > 20;
                }),
                "List selection did not change corresponding marker to red");
        wchar_t content[4096]{};
        SendMessageW(GetDlgItem(app, ID_DETAIL), WM_GETTEXT, 4096,
                     reinterpret_cast<LPARAM>(content));
        require(std::wstring(content) == L"https://example.org/qreader?test=1",
                "Full payload view mismatch");
        SendMessageW(app, WM_COMMAND, ID_COPY, 0);
        require(OpenClipboard(nullptr), "Cannot open copied clipboard");
        auto clip = GetClipboardData(CF_UNICODETEXT);
        auto *text = static_cast<wchar_t *>(GlobalLock(clip));
        bool copied = text && std::wstring(text) == content;
        if (text)
            GlobalUnlock(clip);
        CloseClipboard();
        require(copied, "Copied payload mismatch");
        std::cout
            << "PASS result list, black/green indicator, red selection, full text, clipboard\n";

        RECT initialBounds{};
        GetWindowRect(app, &initialBounds);
        SetWindowPos(app, nullptr, 0, 0, 800, 540, SWP_NOMOVE | SWP_NOZORDER);
        for (const auto &language : loc::languages()) {
            chooseLanguage(language.tag);
            GetWindowTextW(app, title, 256);
            require(std::wstring(title) == loc::text("title"), "Live title translation failed");
            require(SendMessageW(list, LVM_GETITEMCOUNT, 0, 0) == 2,
                    "Language switch lost results");
            require(ListView_GetNextItem(list, -1, LVNI_SELECTED) == 0,
                    "Language switch lost selection");
            wchar_t payload[4096]{};
            SendMessageW(GetDlgItem(app, ID_DETAIL), WM_GETTEXT, 4096,
                         reinterpret_cast<LPARAM>(payload));
            require(std::wstring(payload) == content, "Language switch altered decoded payload");
            require(!(GetWindowLongPtrW(list, GWL_EXSTYLE) & WS_EX_LAYOUTRTL),
                    "Raw payload list must not mirror glyphs");
            RECT client{};
            GetClientRect(app, &client);
            std::vector<RECT> buttons;
            for (int id : {ID_OPEN, ID_CAPTURE, ID_PASTE, ID_COPY}) {
                RECT rect{};
                GetWindowRect(GetDlgItem(app, id), &rect);
                MapWindowPoints(nullptr, app, reinterpret_cast<POINT *>(&rect), 2);
                require(rect.left >= 0 && rect.top >= 0 && rect.right <= client.right &&
                            rect.bottom <= client.bottom,
                        "Translated button outside window");
                for (const auto &other : buttons) {
                    RECT overlap{};
                    require(!IntersectRect(&overlap, &rect, &other), "Translated buttons overlap");
                }
                buttons.push_back(rect);
            }
            RECT positions{}, results{};
            GetWindowRect(indicator, &positions);
            GetWindowRect(list, &results);
            require(language.rtl ? positions.left > results.right : positions.right < results.left,
                    "Language layout direction mismatch");
        }
        SetWindowPos(app, nullptr, 0, 0, initialBounds.right - initialBounds.left,
                     initialBounds.bottom - initialBounds.top, SWP_NOMOVE | SWP_NOZORDER);
        chooseLanguage("zh-CN");
        std::cout << "PASS all 27 live languages at minimum window size, RTL layout, button "
                     "bounds, selection and payload preservation\n";

        const auto singleImage = qr::loadImage(std::filesystem::u8path("fixtures/二维码 空格.png"));
        std::vector<unsigned char> encoded;
        cv::imencode(".png", singleImage, encoded);
        clipboardBytes(app, RegisterClipboardFormatW(L"PNG"), encoded);
        // Exercise the public button; other cases below use the menu command.
        SendMessageW(GetDlgItem(app, ID_PASTE), BM_CLICK, 0, 0);
        chooseLanguage("fr"); // Change while the worker is analyzing the clipboard.
        require(waitFor([&] { return ready() && SendMessageW(list, LVM_GETITEMCOUNT, 0, 0) == 1; }),
                "PNG clipboard recognition failed");
        require(statusText(app, process.hProcess).find(L"Détectés") != std::wstring::npos,
                "Worker completed in stale UI language");
        chooseLanguage("zh-CN");
        cv::imencode(".bmp", qr::loadImage("fixtures/multi.png"), encoded);
        clipboardBytes(app, CF_DIB, {encoded.begin() + 14, encoded.end()});
        SendMessageW(app, WM_COMMAND, ID_PASTE, 0);
        if (!waitFor([&] { return ready() && SendMessageW(list, LVM_GETITEMCOUNT, 0, 0) == 2; })) {
            std::wcerr << L"DIB status: " << statusText(app, process.hProcess) << L'\n';
            throw std::runtime_error("DIB clipboard recognition failed");
        }
        const auto imagePath =
            std::filesystem::absolute(std::filesystem::u8path("fixtures/二维码 空格.png"))
                .wstring();
        DROPFILES drop{};
        drop.pFiles = sizeof(drop);
        drop.fWide = TRUE;
        std::vector<unsigned char> files(sizeof(drop) + (imagePath.size() + 2) * sizeof(wchar_t),
                                         0);
        std::memcpy(files.data(), &drop, sizeof(drop));
        std::memcpy(files.data() + sizeof(drop), imagePath.c_str(),
                    (imagePath.size() + 1) * sizeof(wchar_t));
        clipboardBytes(app, CF_HDROP, files);
        SetForegroundWindow(app);
        keybd_event(VK_CONTROL, 0, 0, 0);
        keybd_event('V', 0, 0, 0);
        keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
        require(waitFor([&] { return ready() && SendMessageW(list, LVM_GETITEMCOUNT, 0, 0) == 1; }),
                "Ctrl+V clipboard image file recognition failed");
        std::cout << "PASS clipboard PNG, DIB, Unicode image file, button and Ctrl+V\n";

        const DWORD clipboardBeforeCancel = GetClipboardSequenceNumber();
        for (auto cancel : {UINT(WM_KEYDOWN), UINT(WM_RBUTTONDOWN)}) {
            SendMessageW(app, WM_COMMAND, ID_CAPTURE, 0);
            HWND overlay = nullptr;
            require(waitFor([&] {
                        overlay = FindWindowW(L"QReader.Capture", nullptr);
                        return overlay && IsWindowVisible(overlay);
                    }),
                    "Capture overlay not shown");
            require(!IsWindowVisible(app), "Main window was not hidden for capture");
            SendMessageW(overlay, cancel, VK_ESCAPE, 0);
            require(waitFor(ready), "Capture cancel did not restore main window");
        }
        require(GetClipboardSequenceNumber() == clipboardBeforeCancel,
                "Cancelled capture changed clipboard");
        std::cout << "PASS capture entry, Esc and right-click cancellation\n";

        // A real visible standard STATIC window supplies known pixels to BitBlt.
        auto pixels = qr::loadImage(std::filesystem::u8path("fixtures/二维码 空格.png"));
        cv::Mat bgra;
        cv::cvtColor(pixels, bgra, cv::COLOR_GRAY2BGRA);
        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = bgra.cols;
        info.bmiHeader.biHeight = -bgra.rows;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        void *bits = nullptr;
        HDC dc = GetDC(nullptr);
        bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
        ReleaseDC(nullptr, dc);
        require(bitmap != nullptr, "Cannot allocate fixture bitmap");
        std::memcpy(bits, bgra.data, bgra.total() * bgra.elemSize());
        fixture = CreateWindowExW(0, L"STATIC", L"QReader test QR", WS_POPUP | SS_BITMAP, 60, 60,
                                  bgra.cols, bgra.rows, nullptr, nullptr, GetModuleHandleW(nullptr),
                                  nullptr);
        require(fixture != nullptr, "Cannot create screenshot fixture window");
        SendMessageW(fixture, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(bitmap));
        ShowWindow(fixture, SW_SHOW);
        UpdateWindow(fixture);
        SetForegroundWindow(app);
        SendMessageW(app, WM_COMMAND, ID_CAPTURE, 0);
        HWND overlay = nullptr;
        require(waitFor([&] {
                    overlay = FindWindowW(L"QReader.Capture", nullptr);
                    return overlay && IsWindowVisible(overlay);
                }),
                "Window capture overlay not shown");
        RECT bounds{};
        GetWindowRect(fixture, &bounds);
        mouse(overlay, WM_MOUSEMOVE, bounds.left + 50, bounds.top + 60);
        mouse(overlay, WM_LBUTTONDOWN, bounds.left + 50, bounds.top + 60);
        mouse(overlay, WM_LBUTTONUP, bounds.left + 50, bounds.top + 60);
        require(waitFor([&] { return ready() && SendMessageW(list, LVM_GETITEMCOUNT, 0, 0) == 1; }),
                "Clicked window screenshot was not decoded");
        auto copiedWindow = qr::decodeClipboardImage(qr::readClipboardImage(fixture));
        require(copiedWindow.cols == bounds.right - bounds.left &&
                    copiedWindow.rows == bounds.bottom - bounds.top,
                "Window screenshot was not copied at original dimensions");
        std::cout << "PASS top-level window screenshot -> OpenCV -> ZXing\n";

        SendMessageW(app, WM_COMMAND, ID_CAPTURE, 0);
        require(waitFor([&] {
                    overlay = FindWindowW(L"QReader.Capture", nullptr);
                    return overlay && IsWindowVisible(overlay);
                }),
                "Drag capture overlay not shown");
        POINT origin{0, 0};
        ClientToScreen(fixture, &origin);
        mouse(overlay, WM_LBUTTONDOWN, origin.x + bgra.cols, origin.y + bgra.rows);
        mouse(overlay, WM_MOUSEMOVE, origin.x, origin.y);
        mouse(overlay, WM_LBUTTONUP, origin.x, origin.y);
        require(waitFor([&] { return ready() && SendMessageW(list, LVM_GETITEMCOUNT, 0, 0) == 1; }),
                "Reverse drag screenshot was not decoded");
        const auto copiedDrag = qr::decodeClipboardImage(qr::readClipboardImage(fixture));
        if (copiedDrag.size() != bgra.size() || cv::norm(copiedDrag, bgra, cv::NORM_INF) != 0) {
            cv::imwrite("clipboard-drag-actual.png", copiedDrag);
            cv::imwrite("clipboard-drag-expected.png", bgra);
            std::cerr << "Clipboard drag actual " << copiedDrag.size() << " type "
                      << copiedDrag.type() << "; expected " << bgra.size() << " type "
                      << bgra.type() << '\n';
        }
        require(copiedDrag.size() == bgra.size() && cv::norm(copiedDrag, bgra, cv::NORM_INF) == 0,
                "Clipboard screenshot pixels differ from dragged area");
        SendMessageW(app, WM_COMMAND, ID_PASTE, 0);
        require(waitFor([&] { return ready() && SendMessageW(list, LVM_GETITEMCOUNT, 0, 0) == 1; }),
                "Copied screenshot cannot be recognized again");
        std::cout << "PASS reverse rectangle screenshot -> OpenCV -> ZXing\n";

        DestroyWindow(fixture);
        fixture = CreateWindowExW(0, L"STATIC", L"QReader nested QR", WS_OVERLAPPEDWINDOW, 60, 60,
                                  440, 440, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        require(fixture != nullptr, "Cannot create nested fixture window");
        HWND panel = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 20, 20, 320, 320,
                                     fixture, nullptr, nullptr, nullptr);
        HWND picture = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_BITMAP, 25, 30,
                                       bgra.cols, bgra.rows, panel, nullptr, nullptr, nullptr);
        require(panel && picture, "Cannot create nested QR control");
        SendMessageW(picture, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(bitmap));
        ShowWindow(fixture, SW_SHOW);
        UpdateWindow(fixture);
        SetForegroundWindow(app);
        SendMessageW(app, WM_COMMAND, ID_CAPTURE, 0);
        require(waitFor([&] {
                    overlay = FindWindowW(L"QReader.Capture", nullptr);
                    return overlay && IsWindowVisible(overlay);
                }),
                "Child capture overlay not shown");
        GetWindowRect(picture, &bounds);
        mouse(overlay, WM_MOUSEMOVE, bounds.left + 60, bounds.top + 70);
        require(waitFor([&] {
                    POINT point{bounds.left + 1, bounds.top + 10};
                    ScreenToClient(overlay, &point);
                    HDC overlayDC = GetDC(overlay);
                    const auto pixel = GetPixel(overlayDC, point.x, point.y);
                    ReleaseDC(overlay, overlayDC);
                    return pixel == RGB(0, 255, 0);
                }),
                "Green border does not follow nested child bounds");
        mouse(overlay, WM_LBUTTONDOWN, bounds.left + 60, bounds.top + 70);
        mouse(overlay, WM_LBUTTONUP, bounds.left + 60, bounds.top + 70);
        require(waitFor([&] { return ready() && SendMessageW(list, LVM_GETITEMCOUNT, 0, 0) == 1; }),
                "Child window QR not decoded");
        const auto dimensions = std::to_wstring(bgra.cols) + L" × " + std::to_wstring(bgra.rows);
        require(statusText(app, process.hProcess).find(L"原图 " + dimensions) != std::wstring::npos,
                "Child capture contains parent window pixels");
        std::cout << "PASS nested child green border, exact child capture dimensions and QR "
                     "recognition\n";
        if (GetSystemMetrics(SM_CXSCREEN) >= 2250 && GetSystemMetrics(SM_CYSCREEN) >= 1500) {
            DestroyWindow(fixture);
            DeleteObject(bitmap);
            bitmap = nullptr;
            const auto large = qr::loadImage("fixtures/large-window.png");
            cv::cvtColor(large, bgra, cv::COLOR_GRAY2BGRA);
            info.bmiHeader.biWidth = bgra.cols;
            info.bmiHeader.biHeight = -bgra.rows;
            dc = GetDC(nullptr);
            bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
            ReleaseDC(nullptr, dc);
            require(bitmap != nullptr, "Cannot allocate large-window bitmap");
            std::memcpy(bits, bgra.data, bgra.total() * bgra.elemSize());
            fixture = CreateWindowExW(0, L"STATIC", L"QReader large-window QR",
                                      WS_POPUP | SS_BITMAP, 20, 20, bgra.cols, bgra.rows, nullptr,
                                      nullptr, GetModuleHandleW(nullptr), nullptr);
            require(fixture != nullptr, "Cannot create large-window fixture");
            SendMessageW(fixture, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(bitmap));
            ShowWindow(fixture, SW_SHOW);
            UpdateWindow(fixture);
            SetForegroundWindow(app);
            SendMessageW(app, WM_COMMAND, ID_CAPTURE, 0);
            require(waitFor([&] {
                        overlay = FindWindowW(L"QReader.Capture", nullptr);
                        return overlay && IsWindowVisible(overlay);
                    }),
                    "Large-window capture overlay not shown");
            mouse(overlay, WM_MOUSEMOVE, 100, 100);
            mouse(overlay, WM_LBUTTONDOWN, 100, 100);
            mouse(overlay, WM_LBUTTONUP, 100, 100);
            require(
                waitFor([&] { return ready() && SendMessageW(list, LVM_GETITEMCOUNT, 0, 0) == 1; }),
                "2154x1380 window small QR was not detected");
            require(statusText(app, process.hProcess).find(L"2154 × 1380") != std::wstring::npos,
                    "Large capture dimensions changed");
            const auto copiedLarge = qr::decodeClipboardImage(qr::readClipboardImage(fixture));
            require(copiedLarge.size() == bgra.size() &&
                        cv::norm(copiedLarge, bgra, cv::NORM_INF) == 0,
                    "Large screenshot clipboard was resized or changed");
            const auto capturedResult = qr::analyze(copiedLarge);
            require(capturedResult.codes.size() == 1 && capturedResult.codes[0].decoded,
                    "Large screenshot clipboard payload cannot decode");
            std::cout
                << "PASS 2154x1380 window screenshot small QR and original clipboard pixels\n";
        }
        if (originalLanguage.empty())
            SendMessageW(app, WM_COMMAND, ID_LANGUAGE_AUTO, 0);
        else
            chooseLanguage(originalLanguage);
        SendMessageW(app, WM_CLOSE, 0, 0);
        require(waitFor([&] { return WaitForSingleObject(process.hProcess, 0) == WAIT_OBJECT_0; }),
                "Application did not exit");
        DWORD code = 1;
        GetExitCodeProcess(process.hProcess, &code);
        require(code == 0, "Application failed on exit");
        DestroyWindow(fixture);
        DeleteObject(bitmap);
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        return 0;
    } catch (const std::exception &e) {
        qr::i18n::select(originalLanguage);
        std::cerr << "FAIL GUI: " << e.what() << '\n';
        if (app)
            PostMessageW(app, WM_CLOSE, 0, 0);
        if (process.hProcess) {
            if (WaitForSingleObject(process.hProcess, 2000) != WAIT_OBJECT_0)
                TerminateProcess(process.hProcess, 1);
            CloseHandle(process.hThread);
            CloseHandle(process.hProcess);
        }
        if (fixture)
            DestroyWindow(fixture);
        if (bitmap)
            DeleteObject(bitmap);
        return 1;
    }
}
