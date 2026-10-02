# QReader — اردو

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [زبان](../README.md#qreader)

Windows 7 اور بعد کے نسخوں کے لیے QR کوڈ ریڈر۔ Windows کے معیاری کنٹرولز سے اسکرین شاٹس، تصویری فائلیں اور کلپ بورڈ کی تصاویر پڑھیں۔ OpenCV مقام تلاش کرتا ہے اور ZXing مواد پڑھتا ہے۔

## ڈاؤن لوڈ اور استعمال

[GitHub Releases](https://github.com/halfstay/qreader/releases)

ZIP کھول کر qreader.exe چلائیں؛ libZXing.dll اسی فولڈر میں رکھیں۔ ونڈو یا کنٹرول پر ماؤس لے جا کر کلک کریں یا علاقہ منتخب کرنے کے لیے گھسیٹیں۔ Esc یا دایاں کلک منسوخ کرتا ہے۔ تصویر اصل حجم میں کلپ بورڈ پر نقل ہوتی ہے۔

| فائل | ⌨ |
| --- | --- |
| تصویر کھولیں — BMP / JPG / PNG | Ctrl+O |
| اسکرین کی تصویر لیں | F6 |
| کلپ بورڈ پڑھیں | Ctrl+V |
| متن کاپی کریں | Ctrl+C |

## زبان اور شناخت

پہلی بار Windows کی زبان استعمال ہوتی ہے۔ مینو سے دوبارہ چلائے بغیر زبان بدلیں؛ نتائج برقرار رہتے ہیں اور ترتیب دائیں سے بائیں ہوتی ہے۔ 26 زبانوں کے 27 تراجم CLDR کے 257 میں سے 209 علاقوں (81.3%) کا احاطہ کرتے ہیں؛ یہ علاقوں کی گنتی ہے، آبادی کا تناسب نہیں۔

زبان → سسٹم کی زبان استعمال کریں

سبز مربع مقامات دکھاتے ہیں اور منتخب نتیجہ سرخ ہوتا ہے۔ تصویر زیادہ سے زیادہ 1920×1080 تک چھوٹی کی جاتی ہے؛ چھوٹے کوڈ اصل پکسل والے حصوں میں بھی تلاش ہوتے ہیں۔ مخصوص پلیٹ فارم کے شعاعی کوڈ شاید معاونت یافتہ نہ ہوں۔

## بلڈ، جانچ اور ریلیز

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
