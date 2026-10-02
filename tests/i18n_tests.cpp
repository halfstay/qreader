#include "i18n.h"
#include "error.h"
#include <iostream>
#include <stdexcept>

static void require(bool condition, const char *reason)
{
    if (!condition)
        throw std::runtime_error(reason);
}
int main()
{
    try {
        namespace loc = qr::i18n;
        loc::initialize(GetModuleHandleW(nullptr));
        struct Restore {
            std::string value;
            ~Restore()
            {
                qr::i18n::select(value);
            }
        } restore{loc::preference()};
        require(loc::select("fr"), "Cannot save language preference");
        loc::initialize(GetModuleHandleW(nullptr));
        require(loc::preference() == "fr" && loc::current().tag == "fr",
                "Language preference did not survive initialization");
        require(loc::match({L"zh-HK"}) == "zh-TW", "Traditional Chinese region match");
        require(loc::match({L"zh-Hans-HK"}) == "zh-CN", "Explicit Chinese script precedence");
        require(loc::match({L"zh-Hant-CN"}) == "zh-TW", "Traditional script precedence");
        require(loc::match({L"pt-PT"}) == "pt", "Regional language fallback");
        require(loc::match({L"xx-XX", L"fr-CA", L"en-US"}) == "fr", "Preferred UI language order");
        require(loc::match({L"xx-XX"}) == "en", "Unsupported UI language fallback");
        const auto english = loc::catalog("en");
        for (const auto &language : loc::languages()) {
            require(loc::select(language.tag, false), "Catalog switch failed");
            const auto entries = loc::catalog(language.tag);
            require(entries.size() == english.size(), "Incomplete language");
            require(std::wstring(loc::text("title")) == entries.at("title"),
                    "Stale active language");
            require(loc::format("error.analysis", {{L"error", L"{error}"}}).find(L"{error}") !=
                        std::wstring::npos,
                    "Inserted text was interpreted as a format string");
            require(loc::exceptionText(qr::Error("error.clipboard_busy")) ==
                        entries.at("error.clipboard_busy"),
                    "Error not localized");
        }
        require(loc::select("ar", false) && loc::current().rtl, "Arabic layout direction");
        require(loc::select("ur", false) && loc::current().rtl, "Urdu layout direction");
        require(loc::select("en", false) && !loc::current().rtl, "LTR direction not restored");
        require(!loc::select("unsupported", false) && loc::current().tag == "en",
                "Invalid preference replaced active language");
        loc::select("", false);
        require(loc::preference().empty() && loc::current().tag == loc::systemLanguage(),
                "Follow-system mode");
        std::cout << "PASS locale resolution, all embedded catalogs, live switching, error keys "
                     "and RTL\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "FAIL i18n: " << error.what() << '\n';
        return 1;
    }
}
