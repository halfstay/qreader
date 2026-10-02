#include "error.h"
#include "win_util.h"
#include "i18n.h"
#include <commctrl.h>
#include <cstring>
#include <stdexcept>

namespace qr
{
void showError(HWND owner, const std::wstring &text)
{
    TASKDIALOGCONFIG dialog{};
    dialog.cbSize = sizeof(dialog);
    dialog.hwndParent = owner;
    dialog.pszWindowTitle = L"QReader";
    dialog.pszContent = text.c_str();
    dialog.pszMainIcon = TD_ERROR_ICON;
    dialog.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
    if (i18n::current().rtl)
        dialog.dwFlags |= TDF_RTL_LAYOUT;
    TASKDIALOG_BUTTON ok{IDOK, i18n::text("ok")};
    dialog.cButtons = 1;
    dialog.pButtons = &ok;
    dialog.nDefaultButton = IDOK;
    TaskDialogIndirect(&dialog, nullptr, nullptr, nullptr);
}
std::wstring wide(const std::string &utf8)
{
    if (utf8.empty())
        return {};
    int count =
        MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring output(count, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), output.data(),
                        count);
    return output;
}
std::wstring displayText(const std::string &utf8, bool singleLine)
{
    auto text = wide(utf8);
    std::wstring output;
    for (wchar_t ch : text) {
        if (ch == L'\0')
            output += L"␀";
        else if (singleLine && (ch == L'\r' || ch == L'\n' || ch == L'\t'))
            output += L' ';
        else if (ch == L'\n') {
            if (output.empty() || output.back() != L'\r')
                output += L'\r';
            output += ch;
        } else
            output += ch;
    }
    return output;
}
void copyText(HWND owner, const std::wstring &text)
{
    const auto bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!memory)
        throw qr::Error("error.memory");
    void *target = GlobalLock(memory);
    if (!target) {
        GlobalFree(memory);
        throw qr::Error("error.clipboard_write");
    }
    std::memcpy(target, text.c_str(), bytes);
    GlobalUnlock(memory);
    if (!OpenClipboard(owner)) {
        GlobalFree(memory);
        throw qr::Error("error.clipboard_busy");
    }
    const bool success = EmptyClipboard() && SetClipboardData(CF_UNICODETEXT, memory);
    CloseClipboard();
    if (!success) {
        GlobalFree(memory);
        throw qr::Error("error.clipboard_write");
    }
}
int windowDpi(HWND window)
{
    using GetDpi = UINT(WINAPI *)(HWND);
    const auto address = GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow");
    GetDpi proc = nullptr;
    static_assert(sizeof(proc) == sizeof(address));
    std::memcpy(&proc, &address, sizeof(proc));
    if (proc)
        return static_cast<int>(proc(window));
    HDC dc = GetDC(window);
    const int dpi = dc ? GetDeviceCaps(dc, LOGPIXELSX) : 96;
    if (dc)
        ReleaseDC(window, dc);
    return dpi;
}
} // namespace qr
