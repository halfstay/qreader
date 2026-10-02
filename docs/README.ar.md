# QReader — العربية

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [اللغة](../README.md#qreader)

قارئ رموز QR لنظام Windows 7 والإصدارات الأحدث، باستخدام عناصر تحكم Windows الأصلية. يحلل لقطات الشاشة وملفات الصور وصور الحافظة. يحدد OpenCV مواضع الرموز ويفك ZXing محتواها.

## التنزيل والاستخدام

[GitHub Releases](https://github.com/halfstay/qreader/releases)

نزّل ملف ZIP وفك ضغطه ثم شغّل qreader.exe مع إبقاء libZXing.dll في المجلد نفسه. حرّك المؤشر فوق نافذة أو عنصر وانقر لالتقاطه، أو اسحب لتحديد منطقة. للإلغاء اضغط Esc أو زر الفأرة الأيمن. تُنسخ اللقطة إلى الحافظة بحجمها الأصلي.

| ملف | ⌨ |
| --- | --- |
| فتح صورة — BMP / JPG / PNG | Ctrl+O |
| التقاط الشاشة | F6 |
| قراءة الحافظة | Ctrl+V |
| نسخ النص | Ctrl+C |

## اللغة والتعرّف

تتبع اللغة Windows عند التشغيل الأول. يمكنك تغييرها من القائمة دون إعادة التشغيل أو فقدان النتائج، مع ترتيب الواجهة من اليمين إلى اليسار. تتوفر 27 مجموعة ترجمة لـ26 لغة، وتغطي 209 من 257 منطقة في CLDR ‏(81.3%) بحسب عدد المناطق لا السكان.

اللغة → اتباع لغة النظام

تشير المربعات الخضراء إلى المواضع ويظهر المحدد بالأحمر. تُصغّر الصور إلى حد أقصى 1920×1080 مع فحص مناطق بالبكسلات الأصلية للرموز الصغيرة. قد لا تُدعم الرموز الشعاعية الخاصة ببعض المنصات.

## البناء والاختبارات والإصدارات

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
