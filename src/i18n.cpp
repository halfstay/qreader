#include "i18n.h"
#include "error.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace qr::i18n
{
namespace
{
HINSTANCE module = nullptr;
std::vector<Language> available;
std::map<std::string, std::wstring> english, active;
std::string selected;
size_t activeIndex = 0;
constexpr wchar_t settingsKey[] = L"Software\\QReader";

std::wstring wide(const std::string &input)
{
    if (input.empty())
        return {};
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input.data(),
                                           static_cast<int>(input.size()), nullptr, 0);
    if (!length)
        throw std::runtime_error("Invalid UTF-8 localization resource.");
    std::wstring output(length, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input.data(), static_cast<int>(input.size()),
                        output.data(), length);
    return output;
}
std::string resource(int id)
{
    auto info = FindResourceW(module, MAKEINTRESOURCEW(id), RT_RCDATA);
    if (!info)
        throw std::runtime_error("Missing localization resource.");
    auto handle = LoadResource(module, info);
    const auto *data = static_cast<const char *>(LockResource(handle));
    if (!data)
        throw std::runtime_error("Cannot load localization resource.");
    return {data, SizeofResource(module, info)};
}
std::string normalize(const std::wstring &input)
{
    std::string output;
    for (wchar_t c : input) {
        if (c > 127)
            return {};
        output += c == L'_' ? '-' : static_cast<char>(c >= L'A' && c <= L'Z' ? c + 32 : c);
    }
    return output;
}
std::string findMatch(const std::wstring &value)
{
    auto name = normalize(value);
    if (name.empty())
        return {};
    auto base = name.substr(0, name.find('-'));
    if (base == "in")
        base = "id";
    if (base == "zh") {
        const bool traditional = name.find("-hant") != std::string::npos ||
                                 (name.find("-hans") == std::string::npos &&
                                  (name == "zh-tw" || name == "zh-hk" || name == "zh-mo"));
        return traditional ? "zh-TW" : "zh-CN";
    }
    for (const auto &lang : available)
        if (name == normalize(wide(lang.tag)) || base == lang.tag)
            return lang.tag;
    return {};
}
void setThreadLanguage(const Language &language)
{
    // Both APIs are available on Windows 7. OS-owned picker chrome still
    // depends on the Windows language packs installed on the machine.
    const LCID locale = LocaleNameToLCID(language.locale.c_str(), 0);
    if (locale)
        SetThreadUILanguage(LANGIDFROMLCID(locale));
    std::wstring names = language.locale + L'\0';
    names += L'\0';
    ULONG count = 0;
    SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, names.c_str(), &count);
}
} // namespace

const std::vector<Language> &languages()
{
    return available;
}
const Language &current()
{
    return available.at(activeIndex);
}
std::string preference()
{
    return selected;
}
std::map<std::string, std::wstring> catalog(const std::string &tag)
{
    const auto found = std::find_if(available.begin(), available.end(),
                                    [&](const auto &lang) { return lang.tag == tag; });
    if (found == available.end())
        throw std::runtime_error("Unknown language.");
    std::map<std::string, std::wstring> result;
    std::istringstream input(resource(found->resource));
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty() || line[0] == '#')
            continue;
        const auto separator = line.find('=');
        if (separator == std::string::npos)
            throw std::runtime_error("Invalid localization entry.");
        std::string value = line.substr(separator + 1), decoded;
        for (size_t i = 0; i < value.size(); ++i) {
            if (value[i] == '\\' && i + 1 < value.size()) {
                const char next = value[++i];
                if (next == 'n')
                    decoded += "\r\n";
                else if (next == 't')
                    decoded += '\t';
                else
                    decoded += next;
            } else
                decoded += value[i];
        }
        if (!result.emplace(line.substr(0, separator), wide(decoded)).second)
            throw std::runtime_error("Duplicate localization entry.");
    }
    return result;
}
std::string match(const std::vector<std::wstring> &preferences)
{
    for (const auto &name : preferences)
        if (auto tag = findMatch(name); !tag.empty())
            return tag;
    return "en";
}
std::string systemLanguage()
{
    ULONG count = 0, size = 0;
    std::vector<std::wstring> preferences;
    GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, nullptr, &size);
    if (size > 1) {
        std::vector<wchar_t> names(size, 0);
        if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, names.data(), &size))
            for (const wchar_t *p = names.data(); *p; p += wcslen(p) + 1)
                preferences.emplace_back(p);
    }
    wchar_t fallback[LOCALE_NAME_MAX_LENGTH]{};
    if (LCIDToLocaleName(MAKELCID(GetUserDefaultUILanguage(), SORT_DEFAULT), fallback,
                         LOCALE_NAME_MAX_LENGTH, 0))
        preferences.emplace_back(fallback);
    return match(preferences);
}
bool select(const std::string &tag, bool persist)
{
    const auto resolved = tag.empty() ? systemLanguage() : tag;
    auto it = std::find_if(available.begin(), available.end(),
                           [&](const auto &lang) { return lang.tag == resolved; });
    if (it == available.end())
        return false;
    auto next = catalog(resolved);
    active.swap(next);
    activeIndex = static_cast<size_t>(it - available.begin());
    selected = tag;
    setThreadLanguage(*it);
    if (persist) {
        HKEY key = nullptr;
        if (RegCreateKeyExW(HKEY_CURRENT_USER, settingsKey, 0, nullptr, 0, KEY_SET_VALUE, nullptr,
                            &key, nullptr) == ERROR_SUCCESS) {
            const auto value = wide(tag);
            RegSetValueExW(key, L"Language", 0, REG_SZ,
                           reinterpret_cast<const BYTE *>(value.c_str()),
                           static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
            RegCloseKey(key);
        }
    }
    return true;
}
void refreshSystem()
{
    if (selected.empty())
        select("", false);
}
void initialize(HINSTANCE instance)
{
    module = instance;
    available.clear();
    std::istringstream input(resource(5000));
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream fields(line);
        std::string tag, name, locale, font, rtl;
        std::getline(fields, tag, '|');
        std::getline(fields, name, '|');
        std::getline(fields, locale, '|');
        std::getline(fields, font, '|');
        std::getline(fields, rtl, '|');
        available.push_back({tag, wide(name), wide(locale), wide(font), rtl == "rtl",
                             5001 + static_cast<int>(available.size())});
    }
    english = catalog("en");
    wchar_t preference[64]{};
    DWORD size = sizeof(preference), type = 0;
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, settingsKey, 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS) {
        if (RegQueryValueExW(key, L"Language", nullptr, &type, reinterpret_cast<BYTE *>(preference),
                             &size) != ERROR_SUCCESS ||
            type != REG_SZ)
            preference[0] = 0;
        preference[63] = 0;
        RegCloseKey(key);
    }
    std::string requested;
    if (*preference)
        for (const auto &lang : available)
            if (wide(lang.tag) == preference)
                requested = lang.tag;
    select(requested, false);
}
const wchar_t *text(const char *key)
{
    auto it = active.find(key);
    if (it != active.end())
        return it->second.c_str();
    it = english.find(key);
    if (it != english.end())
        return it->second.c_str();
    return L"QReader";
}
std::wstring format(const char *key,
                    std::initializer_list<std::pair<std::wstring, std::wstring>> values)
{
    std::wstring output = text(key);
    // Parse placeholders once: user text must never be interpreted as a template.
    std::wstring result;
    for (size_t i = 0; i < output.size();) {
        const auto end = output[i] == L'{' ? output.find(L'}', i) : std::wstring::npos;
        bool replaced = false;
        if (end != std::wstring::npos)
            for (const auto &value : values) {
                if (output.substr(i + 1, end - i - 1) == value.first) {
                    result += value.second;
                    i = end + 1;
                    replaced = true;
                    break;
                }
            }
        if (!replaced)
            result += output[i++];
    }
    return result;
}
std::wstring exceptionText(const std::exception &error)
{
    return text(dynamic_cast<const qr::Error *>(&error) ? error.what() : "error.generic");
}
} // namespace qr::i18n
