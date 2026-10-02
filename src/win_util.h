#pragma once
#include <windows.h>
#include <string>

namespace qr
{
std::wstring wide(const std::string &utf8);
std::wstring displayText(const std::string &utf8, bool singleLine = false);
void copyText(HWND owner, const std::wstring &text);
int windowDpi(HWND window);
void showError(HWND owner, const std::wstring &text);
} // namespace qr
