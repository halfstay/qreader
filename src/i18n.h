#pragma once
#include <windows.h>
#include <map>
#include <string>
#include <vector>
#include <initializer_list>
#include <exception>

namespace qr::i18n
{
struct Language {
    std::string tag;
    std::wstring name, locale, font;
    bool rtl = false;
    int resource = 0;
};
void initialize(HINSTANCE instance);
const std::vector<Language> &languages();
const Language &current();
std::string match(const std::vector<std::wstring> &preferences);
std::string systemLanguage();
std::string preference(); // Empty means follow the Windows UI language.
bool select(const std::string &tag, bool persist = true);
void refreshSystem();
const wchar_t *text(const char *key);
std::wstring format(const char *key,
                    std::initializer_list<std::pair<std::wstring, std::wstring>> values);
std::wstring exceptionText(const std::exception &error);
std::map<std::string, std::wstring> catalog(const std::string &tag);
} // namespace qr::i18n
