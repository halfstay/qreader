# QReader — فارسی

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [زبان](../README.md#qreader)

خوانشگر کد QR برای Windows 7 و نسخه‌های جدیدتر با کنترل‌های استاندارد Windows. تصویر صفحه، فایل تصویر و تصاویر کلیپ‌بورد را می‌خواند. OpenCV موقعیت کد را پیدا می‌کند و ZXing محتوای آن را رمزگشایی می‌کند.

## دریافت و استفاده

[GitHub Releases](https://github.com/halfstay/qreader/releases)

فایل ZIP را باز کنید و qreader.exe را اجرا کنید؛ libZXing.dll باید در همان پوشه باشد. نشانگر را روی پنجره یا کنترل ببرید و کلیک کنید، یا برای انتخاب ناحیه بکشید. Esc یا کلیک راست لغو می‌کند. تصویر با اندازهٔ اصلی در کلیپ‌بورد کپی می‌شود.

| فایل | ⌨ |
| --- | --- |
| باز کردن تصویر — BMP / JPG / PNG | Ctrl+O |
| گرفتن تصویر صفحه | F6 |
| خواندن کلیپ‌بورد | Ctrl+V |
| کپی متن | Ctrl+C |

## زبان و شناسایی

در اجرای نخست از زبان Windows پیروی می‌کند. تغییر زبان از منو بدون راه‌اندازی مجدد و با حفظ نتایج انجام می‌شود و چیدمان راست‌به‌چپ است. ۲۷ مجموعهٔ ترجمه برای ۲۶ زبان، ۲۰۹ از ۲۵۷ منطقهٔ CLDR ‏(۸۱٫۳٪) را پوشش می‌دهند؛ معیار شمار مناطق است، نه جمعیت.

زبان → پیروی از زبان سیستم

مربع‌های سبز موقعیت‌ها را نشان می‌دهند و انتخاب به رنگ قرمز درمی‌آید. تصویر حداکثر به 1920×1080 کوچک می‌شود؛ کدهای کوچک در نواحی پیکسل اصلی نیز جست‌وجو می‌شوند. کدهای شعاعی اختصاصی ممکن است پشتیبانی نشوند.

## ساخت، آزمون و انتشار

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
