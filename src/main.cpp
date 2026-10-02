#include "error.h"
#include "i18n.h"
#include "analyzer.h"
#include "capture.h"
#include "clipboard.h"
#include "win_util.h"
#include "resource.h"
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <thread>

namespace
{
namespace loc = qr::i18n;
const wchar_t *tr(const char *key)
{
    return loc::text(key);
}
constexpr UINT analysisDone = WM_APP + 1;
constexpr UINT_PTR captureTimer = 1;
constexpr wchar_t mainClass[] = L"QReader.Main";
struct WorkResult {
    qr::Analysis analysis;
    std::exception_ptr error;
};

LRESULT CALLBACK aboutProcedure(HWND window, UINT message, WPARAM wp, LPARAM lp, UINT_PTR id,
                                DWORD_PTR)
{
    // Some common-controls implementations do not honor cancellation when
    // IDOK is the only button. Give Esc and the close button the same action.
    if (message == WM_CLOSE || (message == WM_COMMAND && LOWORD(wp) == IDCANCEL)) {
        SendMessageW(window, TDM_CLICK_BUTTON, IDOK, 0);
        return 0;
    }
    if (message == WM_NCDESTROY)
        RemoveWindowSubclass(window, aboutProcedure, id);
    return DefSubclassProc(window, message, wp, lp);
}

HRESULT CALLBACK aboutCallback(HWND window, UINT notification, WPARAM, LPARAM data, LONG_PTR)
{
    if (notification == TDN_CREATED)
        SetWindowSubclass(window, aboutProcedure, 1, 0);
    if (notification == TDN_HYPERLINK_CLICKED) {
        const auto result = ShellExecuteW(window, L"open", reinterpret_cast<const wchar_t *>(data),
                                          nullptr, nullptr, SW_SHOWNORMAL);
        if (reinterpret_cast<INT_PTR>(result) <= 32)
            qr::showError(window, tr("error.link"));
    }
    return S_OK;
}

class App
{
  public:
    explicit App(HINSTANCE instance) : instance_(instance), capture_(instance) {}
    ~App()
    {
        if (worker_.joinable())
            worker_.join();
        if (font_)
            DeleteObject(font_);
    }
    int run(int show, const std::wstring &initialPath);

  private:
    static LRESULT CALLBACK procedure(HWND, UINT, WPARAM, LPARAM);
    LRESULT dispatch(UINT, WPARAM, LPARAM);
    HWND control(const wchar_t *cls, const wchar_t *text, DWORD style, int id, DWORD ex = 0);
    void createControls();
    void layout();
    void applyLanguage();
    void updateStatus();
    void updateResultText();
    void updateFont();
    int px(int value) const
    {
        return MulDiv(value, dpi_, 96);
    }
    void status(const char *key)
    {
        statusKey_ = key;
        updateStatus();
    }
    void error(const std::wstring &text)
    {
        qr::showError(window_, text);
    }
    void enable();
    void openFile();
    void startCapture();
    void pasteImage();
    void startWork(cv::Mat image, std::filesystem::path path = {},
                   std::unique_ptr<qr::ClipboardImage> clipboard = nullptr);
    void complete(WorkResult &work);
    void select();
    void drawIndicator(const DRAWITEMSTRUCT &draw);
    void copy();
    HINSTANCE instance_;
    HWND window_ = nullptr, open_ = nullptr, screen_ = nullptr, paste_ = nullptr, copy_ = nullptr,
         hint_ = nullptr;
    HWND indicatorLabel_ = nullptr, indicator_ = nullptr, listLabel_ = nullptr, list_ = nullptr;
    HWND detail_ = nullptr, status_ = nullptr;
    HFONT font_ = nullptr;
    int dpi_ = 96;
    qr::Capture capture_;
    qr::Analysis analysis_;
    std::thread worker_;
    bool busy_ = false, closing_ = false, capturePending_ = false;
    int selected_ = -1;
    std::string statusKey_ = "ready";
};

HWND App::control(const wchar_t *cls, const wchar_t *text, DWORD style, int id, DWORD ex)
{
    HWND result =
        CreateWindowExW(ex, cls, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 1, 1, window_,
                        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), instance_, nullptr);
    if (!result)
        throw qr::Error("error.window");
    return result;
}
void App::createControls()
{
    open_ = control(L"BUTTON", tr("open"), WS_TABSTOP | BS_PUSHBUTTON, ID_OPEN);
    screen_ = control(L"BUTTON", tr("capture"), WS_TABSTOP | BS_PUSHBUTTON, ID_CAPTURE);
    paste_ = control(L"BUTTON", tr("paste"), WS_TABSTOP | BS_PUSHBUTTON, ID_PASTE);
    copy_ = control(L"BUTTON", tr("copy"), WS_TABSTOP | BS_PUSHBUTTON, ID_COPY);
    hint_ = control(L"STATIC", tr("hint"), SS_LEFT, 0);
    indicatorLabel_ = control(L"STATIC", tr("indicator"), SS_LEFT, 0);
    indicator_ = control(L"STATIC", L"", SS_OWNERDRAW, ID_INDICATOR);
    listLabel_ = control(L"STATIC", tr("results"), SS_LEFT, 0);
    list_ = control(WC_LISTVIEWW, L"", WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                    ID_RESULTS, WS_EX_CLIENTEDGE);
    ListView_SetExtendedListViewStyle(list_, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER |
                                                 LVS_EX_GRIDLINES);
    const wchar_t *names[] = {tr("number"), tr("state"), tr("content")};
    for (int i = 0; i < 3; ++i) {
        LVCOLUMNW col{};
        col.mask = LVCF_TEXT | LVCF_WIDTH;
        col.pszText = const_cast<wchar_t *>(names[i]);
        col.cx = 100;
        ListView_InsertColumn(list_, i, &col);
    }
    detail_ =
        control(L"EDIT", L"", WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                ID_DETAIL, WS_EX_CLIENTEDGE);
    SendMessageW(detail_, EM_SETLIMITTEXT, 1024 * 1024, 0);
    status_ = control(STATUSCLASSNAMEW, L"", SBARS_SIZEGRIP, ID_STATUS);
    applyLanguage();
    enable();
    status("ready");
    DragAcceptFiles(window_, TRUE);
}
void App::updateFont()
{
    dpi_ = qr::windowDpi(window_);
    NONCLIENTMETRICSW metrics{};
    metrics.cbSize = sizeof(metrics);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0);
    LOGFONTW spec = metrics.lfMessageFont;
    spec.lfHeight = -MulDiv(10, dpi_, 72);
    spec.lfCharSet = DEFAULT_CHARSET;
    spec.lfQuality = CLEARTYPE_QUALITY;
    wcsncpy(spec.lfFaceName, loc::current().font.c_str(), LF_FACESIZE - 1);
    spec.lfFaceName[LF_FACESIZE - 1] = 0;
    HFONT next = CreateFontIndirectW(&spec);
    if (!next)
        return;
    for (HWND child = GetWindow(window_, GW_CHILD); child; child = GetWindow(child, GW_HWNDNEXT))
        SendMessageW(child, WM_SETFONT, reinterpret_cast<WPARAM>(next), TRUE);
    if (font_)
        DeleteObject(font_);
    font_ = next;
}
void App::applyLanguage()
{
    SetWindowTextW(window_, tr("title"));
    const std::pair<HWND, const char *> labels[] = {
        {open_, "open"}, {screen_, "capture"},           {paste_, "paste"},      {copy_, "copy"},
        {hint_, "hint"}, {indicatorLabel_, "indicator"}, {listLabel_, "results"}};
    const bool rtl = loc::current().rtl;
    for (const auto &entry : labels) {
        SetWindowTextW(entry.first, tr(entry.second));
        auto style = GetWindowLongPtrW(entry.first, GWL_EXSTYLE);
        SetWindowLongPtrW(entry.first, GWL_EXSTYLE,
                          rtl ? style | WS_EX_RTLREADING : style & ~WS_EX_RTLREADING);
    }
    for (HWND label : {hint_, indicatorLabel_, listLabel_}) {
        const auto style = GetWindowLongPtrW(label, GWL_STYLE) & ~SS_TYPEMASK;
        SetWindowLongPtrW(label, GWL_STYLE, style | (rtl ? SS_RIGHT : SS_LEFT));
    }
    auto style = GetWindowLongPtrW(list_, GWL_EXSTYLE) & ~(WS_EX_LAYOUTRTL | WS_EX_RTLREADING);
    // Arrange native columns explicitly. Mirroring the ListView window can
    // also mirror payload glyphs on some common-control implementations.
    SetWindowLongPtrW(list_, GWL_EXSTYLE, style | (rtl ? WS_EX_RTLREADING : 0));
    SetWindowPos(list_, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    const char *columns[] = {"number", "state", "content"};
    for (int i = 0; i < 3; ++i) {
        LVCOLUMNW column{};
        column.mask = LVCF_TEXT | LVCF_FMT;
        column.pszText = const_cast<wchar_t *>(tr(columns[i]));
        column.fmt = rtl && i != 2 ? LVCFMT_RIGHT : LVCFMT_LEFT;
        ListView_SetColumn(list_, i, &column);
    }
    int order[] = {rtl ? 2 : 0, 1, rtl ? 0 : 2};
    ListView_SetColumnOrderArray(list_, 3, order);
    HMENU menu = CreateMenu(), file = CreatePopupMenu(), language = CreatePopupMenu(),
          help = CreatePopupMenu();
    const auto entry = [&](HMENU target, UINT id, const char *key, const wchar_t *shortcut) {
        const std::wstring label = std::wstring(tr(key)) + shortcut;
        AppendMenuW(target, MF_STRING, id, label.c_str());
    };
    entry(file, ID_OPEN, "open", L"\tCtrl+O");
    entry(file, ID_CAPTURE, "capture", L"\tF6");
    entry(file, ID_PASTE, "paste", L"\tCtrl+V");
    entry(file, ID_COPY, "copy", L"\tCtrl+C");
    AppendMenuW(file, MF_SEPARATOR, 0, nullptr);
    entry(file, ID_EXIT, "menu.exit", L"");
    AppendMenuW(language, MF_STRING | (loc::preference().empty() ? MF_CHECKED : 0),
                ID_LANGUAGE_AUTO, tr("menu.auto"));
    AppendMenuW(language, MF_SEPARATOR, 0, nullptr);
    for (size_t i = 0; i < loc::languages().size(); ++i) {
        const auto &lang = loc::languages()[i];
        AppendMenuW(language,
                    MF_STRING | (i == 14 ? MF_MENUBARBREAK : 0) |
                        (loc::preference() == lang.tag ? MF_CHECKED : 0),
                    ID_LANGUAGE_FIRST + i, lang.name.c_str());
    }
    entry(help, ID_ABOUT, "menu.about", L"");
    const auto top = [&](HMENU popup, const char *key, const wchar_t *accelerator) {
        const std::wstring label = std::wstring(tr(key)) + accelerator;
        AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(popup), label.c_str());
    };
    if (rtl) {
        top(help, "menu.help", L" (&H)");
        top(language, "menu.language", L" (&L)");
        top(file, "menu.file", L" (&F)");
    } else {
        top(file, "menu.file", L" (&F)");
        top(language, "menu.language", L" (&L)");
        top(help, "menu.help", L" (&H)");
    }
    HMENU old = GetMenu(window_);
    SetMenu(window_, menu);
    if (old)
        DestroyMenu(old);
    DrawMenuBar(window_);
    updateFont();
    updateResultText();
    updateStatus();
    enable();
    layout();
}
void App::updateResultText()
{
    SendMessageW(list_, WM_SETREDRAW, FALSE, 0);
    for (size_t i = 0; i < analysis_.codes.size(); ++i) {
        const auto &code = analysis_.codes[i];
        ListView_SetItemText(list_, static_cast<int>(i), 1,
                             const_cast<wchar_t *>(tr(code.decoded ? "decoded" : "undecoded")));
        std::wstring value = code.decoded ? qr::displayText(code.text, true) : tr("unreadable");
        if (value.size() > 512)
            value = value.substr(0, 512) + L"…";
        ListView_SetItemText(list_, static_cast<int>(i), 2, value.data());
    }
    SendMessageW(list_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(list_, nullptr, TRUE);
    select();
}
void App::updateStatus()
{
    std::wstring value;
    if (!statusKey_.empty())
        value = tr(statusKey_.c_str());
    else {
        const auto decoded = std::count_if(analysis_.codes.begin(), analysis_.codes.end(),
                                           [](const auto &code) { return code.decoded; });
        value = loc::format("summary", {{L"width", std::to_wstring(analysis_.original.width)},
                                        {L"height", std::to_wstring(analysis_.original.height)},
                                        {L"found", std::to_wstring(analysis_.codes.size())},
                                        {L"decoded", std::to_wstring(decoded)}});
        if (analysis_.codes.empty())
            value += std::wstring(L" | ") + tr("no_codes");
    }
    SendMessageW(status_, SB_SETTEXTW, loc::current().rtl ? SBT_RTLREADING : 0,
                 reinterpret_cast<LPARAM>(value.c_str()));
}
void App::layout()
{
    if (!list_)
        return;
    RECT client{};
    GetClientRect(window_, &client);
    SendMessageW(status_, WM_SIZE, 0, 0);
    RECT statusRect{};
    GetWindowRect(status_, &statusRect);
    const int margin = px(12), gap = px(12), width = client.right - 2 * margin;
    const bool rtl = loc::current().rtl;
    HDC dc = GetDC(window_);
    const auto previous = SelectObject(dc, font_);
    const auto measure = [&](const wchar_t *value, int available, UINT flags) {
        RECT area{0, 0, available, 0};
        DrawTextW(dc, value, -1, &area, flags | DT_CALCRECT | DT_NOPREFIX);
        return area;
    };
    const auto place = [&](HWND control, int x, int y, int w, int h) {
        MoveWindow(control, rtl ? client.right - x - w : x, y, std::max(1, w), std::max(1, h),
                   TRUE);
    };
    int x = margin, y = margin;
    const int buttonHeight =
        std::max(px(32), static_cast<int>(measure(L"Ag", width, DT_SINGLELINE).bottom) + px(14));
    for (HWND button : {open_, screen_, paste_, copy_}) {
        wchar_t value[256]{};
        GetWindowTextW(button, value, 256);
        const int w = std::min(
            width, std::max(px(110),
                            static_cast<int>(measure(value, width, DT_SINGLELINE).right) + px(28)));
        if (x > margin && x + w > client.right - margin) {
            x = margin;
            y += buttonHeight + px(8);
        }
        place(button, x, y, w, buttonHeight);
        x += w + gap;
    }
    y += buttonHeight + px(10);
    const int hintHeight = measure(tr("hint"), width, DT_WORDBREAK).bottom + px(4);
    place(hint_, margin, y, width, hintHeight);
    y += hintHeight + px(8);
    const int leftWidth = (width - gap) * 42 / 100, rightX = margin + leftWidth + gap,
              rightWidth = width - gap - leftWidth;
    const int labelHeight = std::max(measure(tr("indicator"), leftWidth, DT_WORDBREAK).bottom,
                                     measure(tr("results"), rightWidth, DT_WORDBREAK).bottom) +
                            px(6);
    place(indicatorLabel_, margin, y, leftWidth, labelHeight);
    place(listLabel_, rightX, y, rightWidth, labelHeight);
    y += labelHeight;
    const int bottom = client.bottom - (statusRect.bottom - statusRect.top) - margin;
    const int height = std::max(px(100), bottom - y), detailHeight = std::min(px(142), height / 3);
    place(indicator_, margin, y, leftWidth, height);
    place(list_, rightX, y, rightWidth, height - detailHeight - gap);
    place(detail_, rightX, bottom - detailHeight, rightWidth, detailHeight);
    const int numberWidth = std::max(
        px(48), static_cast<int>(measure(tr("number"), width, DT_SINGLELINE).right) + px(20));
    const int stateWidth =
        std::max({static_cast<int>(measure(tr("state"), width, DT_SINGLELINE).right),
                  static_cast<int>(measure(tr("decoded"), width, DT_SINGLELINE).right),
                  static_cast<int>(measure(tr("undecoded"), width, DT_SINGLELINE).right)}) +
        px(20);
    ListView_SetColumnWidth(list_, 0, numberWidth);
    ListView_SetColumnWidth(list_, 1, stateWidth);
    ListView_SetColumnWidth(list_, 2,
                            std::max(px(120), rightWidth - numberWidth - stateWidth - px(24)));
    SelectObject(dc, previous);
    ReleaseDC(window_, dc);
    InvalidateRect(indicator_, nullptr, FALSE);
}
void App::enable()
{
    EnableWindow(open_, !busy_ && !closing_ && !capturePending_);
    EnableWindow(screen_, !busy_ && !closing_ && !capturePending_);
    EnableWindow(paste_, !busy_ && !closing_ && !capturePending_);
    const bool canCopy = selected_ >= 0 && selected_ < static_cast<int>(analysis_.codes.size()) &&
                         analysis_.codes[selected_].decoded;
    EnableWindow(copy_, canCopy && !closing_);
    if (HMENU menu = GetMenu(window_)) {
        EnableMenuItem(menu, ID_OPEN,
                       MF_BYCOMMAND | ((!busy_ && !closing_) ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(menu, ID_CAPTURE,
                       MF_BYCOMMAND | ((!busy_ && !closing_) ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(menu, ID_PASTE,
                       MF_BYCOMMAND |
                           ((!busy_ && !closing_ && !capturePending_) ? MF_ENABLED : MF_GRAYED));
        EnableMenuItem(menu, ID_COPY,
                       MF_BYCOMMAND | ((canCopy && !closing_) ? MF_ENABLED : MF_GRAYED));
    }
}
void App::openFile()
{
    if (busy_ || capturePending_ || closing_)
        return;
    std::wstring path(32768, L'\0');
    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = window_;
    std::wstring filter = tr("image_filter");
    filter += L'\0';
    filter += L"*.bmp;*.jpg;*.jpeg;*.png";
    filter += L'\0';
    filter += L'\0';
    dialog.lpstrFilter = filter.c_str();
    dialog.lpstrFile = path.data();
    dialog.nMaxFile = static_cast<DWORD>(path.size());
    dialog.lpstrTitle = tr("open_title");
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
    if (GetOpenFileNameW(&dialog)) {
        path.resize(wcslen(path.c_str()));
        startWork({}, path);
    } else if (CommDlgExtendedError() != 0)
        error(tr("error.open_dialog"));
}
void App::startCapture()
{
    if (busy_ || closing_ || capturePending_ || capture_.active())
        return;
    capturePending_ = true;
    enable();
    ShowWindow(window_, SW_HIDE);
    // Let the compositor remove our window before copying the desktop.
    DwmFlush();
    if (!SetTimer(window_, captureTimer, 150, nullptr)) {
        capturePending_ = false;
        ShowWindow(window_, SW_SHOW);
        enable();
        throw qr::Error("error.capture");
    }
}
void App::pasteImage()
{
    if (busy_ || closing_ || capturePending_ || capture_.active())
        return;
    auto input = std::make_unique<qr::ClipboardImage>(qr::readClipboardImage(window_));
    startWork({}, {}, std::move(input));
}
void App::startWork(cv::Mat image, std::filesystem::path path,
                    std::unique_ptr<qr::ClipboardImage> clipboard)
{
    if (busy_ || closing_)
        return;
    if (worker_.joinable())
        worker_.join();
    auto result = std::make_unique<WorkResult>();
    busy_ = true;
    enable();
    status("analyzing");
    try {
        worker_ =
            std::thread([hwnd = window_, image = std::move(image), path = std::move(path),
                         result = std::move(result), clipboard = std::move(clipboard)]() mutable {
                try {
                    result->analysis =
                        qr::analyze(clipboard ? qr::decodeClipboardImage(*clipboard)
                                              : (path.empty() ? image : qr::loadImage(path)));
                } catch (...) {
                    result->error = std::current_exception();
                }
                // The window stays alive until this message is received, including when closing.
                if (PostMessageW(hwnd, analysisDone, 0, reinterpret_cast<LPARAM>(result.get())))
                    result.release();
            });
    } catch (...) {
        busy_ = false;
        enable();
        status("start_failed");
        throw;
    }
}
void App::complete(WorkResult &work)
{
    if (worker_.joinable())
        worker_.join();
    busy_ = false;
    if (closing_) {
        DestroyWindow(window_);
        return;
    }
    if (work.error) {
        std::wstring detail = tr("error.generic");
        try {
            std::rethrow_exception(work.error);
        } catch (const std::exception &ex) {
            detail = loc::exceptionText(ex);
        } catch (...) {
        }
        status("failed");
        enable();
        error(loc::format("error.analysis", {{L"error", detail}}));
        return;
    }
    selected_ = -1;
    ListView_DeleteAllItems(list_);
    analysis_ = std::move(work.analysis);
    SetWindowTextW(detail_, L"");
    SendMessageW(list_, WM_SETREDRAW, FALSE, 0);
    for (size_t i = 0; i < analysis_.codes.size(); ++i) {
        auto number = std::to_wstring(i + 1);
        LVITEMW item{};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(i);
        item.pszText = number.data();
        ListView_InsertItem(list_, &item);
        auto &code = analysis_.codes[i];
        auto state = code.decoded ? tr("decoded") : tr("undecoded");
        ListView_SetItemText(list_, static_cast<int>(i), 1, const_cast<wchar_t *>(state));
        auto text = code.decoded ? qr::displayText(code.text, true) : tr("unreadable");
        if (text.size() > 512)
            text = text.substr(0, 512) + L"…";
        ListView_SetItemText(list_, static_cast<int>(i), 2, text.data());
    }
    SendMessageW(list_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(list_, nullptr, TRUE);
    InvalidateRect(indicator_, nullptr, FALSE);
    enable();
    status("");
}
void App::select()
{
    selected_ = ListView_GetNextItem(list_, -1, LVNI_SELECTED);
    std::wstring text;
    if (selected_ >= 0 && selected_ < static_cast<int>(analysis_.codes.size())) {
        auto &code = analysis_.codes[selected_];
        text = code.decoded ? qr::displayText(code.text) : tr("unreadable");
    }
    SetWindowTextW(detail_, text.c_str());
    InvalidateRect(indicator_, nullptr, FALSE);
    enable();
}
void App::copy()
{
    if (selected_ < 0 || selected_ >= static_cast<int>(analysis_.codes.size()))
        return;
    const auto &code = analysis_.codes[selected_];
    if (code.decoded) {
        qr::copyText(window_, qr::displayText(code.text));
        status("copied");
    }
}
void App::drawIndicator(const DRAWITEMSTRUCT &draw)
{
    const int saved = SaveDC(draw.hDC);
    FillRect(draw.hDC, &draw.rcItem, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
    SetBkMode(draw.hDC, TRANSPARENT);
    SelectObject(draw.hDC, font_);
    if (analysis_.analyzed.width <= 0) {
        SetTextColor(draw.hDC, RGB(140, 140, 140));
        RECT text = draw.rcItem;
        DrawTextW(draw.hDC, tr("empty_hint"), -1, &text,
                  DT_CENTER | DT_WORDBREAK | (loc::current().rtl ? DT_RTLREADING : 0));
        RestoreDC(draw.hDC, saved);
        return;
    }
    auto bounds = qr::fitInside(analysis_.analyzed,
                                {draw.rcItem.left + px(14), draw.rcItem.top + px(14),
                                 draw.rcItem.right - px(14), draw.rcItem.bottom - px(14)});
    RECT frame{bounds.left, bounds.top, bounds.right, bounds.bottom};
    HBRUSH gray = CreateSolidBrush(RGB(65, 65, 65));
    FrameRect(draw.hDC, &frame, gray);
    DeleteObject(gray);
    const double sx = double(bounds.width()) / analysis_.analyzed.width;
    const double sy = double(bounds.height()) / analysis_.analyzed.height;
    // Use the shortest projected edge as the common square size. Keep each
    // marker centered on its code, even for rotated or perspective-skewed codes.
    double shortest = std::min(bounds.width(), bounds.height());
    for (const auto &code : analysis_.codes) {
        for (size_t p = 0; p < code.corners.size(); ++p) {
            const auto edge = code.corners[(p + 1) % code.corners.size()] - code.corners[p];
            shortest = std::min(shortest, std::hypot(edge.x * sx, edge.y * sy));
        }
    }
    const int side = std::max(1, static_cast<int>(std::lround(shortest)));
    SelectObject(draw.hDC, GetStockObject(HOLLOW_BRUSH));
    for (size_t i = 0; i < analysis_.codes.size(); ++i) {
        const COLORREF color = static_cast<int>(i) == selected_ ? RGB(255, 45, 45) : RGB(0, 255, 0);
        HPEN pen = CreatePen(PS_SOLID, std::max(2, px(2)), color);
        auto old = SelectObject(draw.hDC, pen);
        cv::Point2d center{};
        for (const auto &point : analysis_.codes[i].corners)
            center += cv::Point2d(point);
        center *= 0.25;
        const int left = bounds.left + static_cast<int>(std::lround(center.x * sx - side / 2.0));
        const int top = bounds.top + static_cast<int>(std::lround(center.y * sy - side / 2.0));
        Rectangle(draw.hDC, left, top, left + side, top + side);
        SetTextColor(draw.hDC, color);
        auto label = std::to_wstring(i + 1);
        TextOutW(draw.hDC, left + px(3), top + px(3), label.c_str(),
                 static_cast<int>(label.size()));
        SelectObject(draw.hDC, old);
        DeleteObject(pen);
    }
    RestoreDC(draw.hDC, saved);
}
LRESULT CALLBACK App::procedure(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    auto *app = reinterpret_cast<App *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        app = static_cast<App *>(reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams);
        app->window_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }
    if (!app)
        return DefWindowProcW(hwnd, message, wp, lp);
    try {
        return app->dispatch(message, wp, lp);
    } catch (const std::exception &ex) {
        app->error(loc::exceptionText(ex));
        return message == WM_CREATE ? -1 : 0;
    }
}
LRESULT App::dispatch(UINT message, WPARAM wp, LPARAM lp)
{
    switch (message) {
    case WM_CREATE:
        createControls();
        return 0;
    case WM_SIZE:
        layout();
        return 0;
    case WM_GETMINMAXINFO: {
        auto *limits = reinterpret_cast<MINMAXINFO *>(lp);
        limits->ptMinTrackSize = {px(800), px(540)};
        return 0;
    }
    case WM_DPICHANGED: {
        const RECT rect = *reinterpret_cast<RECT *>(lp);
        updateFont();
        SetWindowPos(window_, nullptr, rect.left, rect.top, rect.right - rect.left,
                     rect.bottom - rect.top, SWP_NOZORDER | SWP_NOACTIVATE);
        layout();
        return 0;
    }
    case WM_SETTINGCHANGE:
        if (loc::preference().empty()) {
            loc::refreshSystem();
            applyLanguage();
        }
        return 0;
    case WM_COMMAND:
        if (LOWORD(wp) == ID_LANGUAGE_AUTO ||
            (LOWORD(wp) >= ID_LANGUAGE_FIRST &&
             LOWORD(wp) < ID_LANGUAGE_FIRST + loc::languages().size())) {
            const std::string language = LOWORD(wp) == ID_LANGUAGE_AUTO
                                             ? ""
                                             : loc::languages()[LOWORD(wp) - ID_LANGUAGE_FIRST].tag;
            if (loc::select(language))
                applyLanguage();
            return 0;
        }
        switch (LOWORD(wp)) {
        case ID_OPEN:
            openFile();
            return 0;
        case ID_CAPTURE:
            startCapture();
            return 0;
        case ID_PASTE:
            pasteImage();
            return 0;
        case ID_COPY:
            copy();
            return 0;
        case ID_EXIT:
            SendMessageW(window_, WM_CLOSE, 0, 0);
            return 0;
        case ID_ABOUT: {
            TASKDIALOGCONFIG about{};
            about.cbSize = sizeof(about);
            about.hwndParent = window_;
            about.hInstance = instance_;
            about.pszWindowTitle = tr("about.title");
            const std::wstring aboutText = std::wstring(L"QReader 1.2.0\r\n") +
                                           tr("about.description") +
                                           L"\r\n\r\n"
                                           L"<a "
                                           L"href=\"https://github.com/halfstay/qreader\">https://"
                                           L"github.com/halfstay/qreader</a>";
            about.pszContent = aboutText.c_str();
            about.pszMainIcon = MAKEINTRESOURCEW(ID_ICON);
            about.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_ALLOW_DIALOG_CANCELLATION |
                            TDF_POSITION_RELATIVE_TO_WINDOW;
            if (loc::current().rtl)
                about.dwFlags |= TDF_RTL_LAYOUT;
            TASKDIALOG_BUTTON button{IDOK, tr("ok")};
            about.cButtons = 1;
            about.pButtons = &button;
            about.nDefaultButton = IDOK;
            about.pfCallback = aboutCallback;
            if (FAILED(TaskDialogIndirect(&about, nullptr, nullptr, nullptr)))
                error(tr("error.about"));
            return 0;
        }
        }
        break;
    case WM_NOTIFY: {
        const auto *header = reinterpret_cast<NMHDR *>(lp);
        if (header->idFrom == ID_RESULTS && header->code == LVN_ITEMCHANGED)
            select();
        return 0;
    }
    case WM_DRAWITEM:
        if (wp == ID_INDICATOR) {
            drawIndicator(*reinterpret_cast<DRAWITEMSTRUCT *>(lp));
            return TRUE;
        }
        break;
    case WM_TIMER:
        if (wp == captureTimer) {
            KillTimer(window_, captureTimer);
            capturePending_ = false;
            try {
                capture_.begin(window_, [this](cv::Mat image) {
                    ShowWindow(window_, SW_SHOW);
                    SetForegroundWindow(window_);
                    enable();
                    if (!image.empty()) {
                        std::wstring clipboardError;
                        try {
                            qr::copyScreenshot(window_, image);
                        } catch (const std::exception &ex) {
                            clipboardError = loc::exceptionText(ex);
                        }
                        startWork(std::move(image));
                        if (!clipboardError.empty())
                            error(
                                loc::format("error.clipboard_copy", {{L"error", clipboardError}}));
                    } else
                        status("cancelled");
                });
            } catch (...) {
                ShowWindow(window_, SW_SHOW);
                enable();
                throw;
            }
            return 0;
        }
        break;
    case WM_DROPFILES: {
        HDROP drop = reinterpret_cast<HDROP>(wp);
        const UINT count = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);
        std::wstring path;
        if (count == 1) {
            path.resize(DragQueryFileW(drop, 0, nullptr, 0) + 1);
            DragQueryFileW(drop, 0, path.data(), static_cast<UINT>(path.size()));
            path.resize(wcslen(path.c_str()));
        }
        DragFinish(drop);
        if (!busy_ && !path.empty())
            startWork({}, path);
        else if (count != 1)
            error(tr("error.one_file"));
        return 0;
    }
    case analysisDone: {
        std::unique_ptr<WorkResult> work(reinterpret_cast<WorkResult *>(lp));
        complete(*work);
        return 0;
    }
    case WM_CLOSE:
        KillTimer(window_, captureTimer);
        capturePending_ = false;
        if (capture_.active())
            capture_.cancel();
        if (busy_) {
            closing_ = true;
            enable();
            status("closing");
        } else
            DestroyWindow(window_);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window_, message, wp, lp);
}
int App::run(int show, const std::wstring &initialPath)
{
    WNDCLASSEXW cls{};
    cls.cbSize = sizeof(cls);
    cls.hInstance = instance_;
    cls.lpszClassName = mainClass;
    cls.lpfnWndProc = procedure;
    cls.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    cls.hIcon = static_cast<HICON>(LoadImageW(instance_, MAKEINTRESOURCEW(ID_ICON), IMAGE_ICON,
                                              GetSystemMetrics(SM_CXICON),
                                              GetSystemMetrics(SM_CYICON), LR_SHARED));
    cls.hIconSm = static_cast<HICON>(LoadImageW(instance_, MAKEINTRESOURCEW(ID_ICON), IMAGE_ICON,
                                                GetSystemMetrics(SM_CXSMICON),
                                                GetSystemMetrics(SM_CYSMICON), LR_SHARED));
    if (!cls.hIcon || !cls.hIconSm)
        throw qr::Error("error.window");
    cls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    cls.lpszMenuName = nullptr;
    if (!RegisterClassExW(&cls))
        throw qr::Error("error.window");
    window_ =
        CreateWindowExW(WS_EX_CONTROLPARENT, mainClass, tr("title"), WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, 1040, 720, nullptr, nullptr, instance_, this);
    if (!window_)
        throw qr::Error("error.window");
    SendMessageW(window_, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(cls.hIcon));
    SendMessageW(window_, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(cls.hIconSm));
    ShowWindow(window_, show);
    UpdateWindow(window_);
    if (!initialPath.empty())
        startWork({}, initialPath);
    HACCEL keys = LoadAcceleratorsW(instance_, MAKEINTRESOURCEW(ID_ACCELERATORS));
    MSG message{};
    int result;
    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0) {
        if (!capture_.active() && TranslateAcceleratorW(window_, keys, &message))
            continue;
        if (!capture_.active() && IsDialogMessageW(window_, &message))
            continue;
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return result < 0 ? 1 : static_cast<int>(message.wParam);
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show)
{
    INITCOMMONCONTROLSEX controls{sizeof(controls),
                                  ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&controls);
    cv::setNumThreads(1);
    int argc = 0;
    wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::wstring initialPath = (argv && argc > 1) ? argv[1] : L"";
    if (argv)
        LocalFree(argv);
    try {
        loc::initialize(instance);
        App app(instance);
        return app.run(show, initialPath);
    } catch (const std::exception &) {
        MessageBoxW(nullptr, L"QReader could not start. Check that all program files are present.",
                    L"QReader", MB_OK | MB_ICONERROR);
        return 1;
    }
}
