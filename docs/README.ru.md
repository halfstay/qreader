# QReader — Русский

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Язык](../README.md#qreader)

Программа чтения QR-кодов для Windows 7 и новее с обычными элементами управления Windows. Работает со снимками экрана, файлами изображений и буфером обмена. OpenCV находит коды, ZXing расшифровывает содержимое.

## Загрузка и использование

[GitHub Releases](https://github.com/halfstay/qreader/releases)

Распакуйте ZIP и запустите qreader.exe; libZXing.dll должна находиться рядом. Наведите мышь на окно или элемент и щёлкните либо выделите область перетаскиванием. Esc или правая кнопка отменяет захват. Снимок копируется в буфер обмена в исходном размере.

| Файл | ⌨ |
| --- | --- |
| Открыть изображение — BMP / JPG / PNG | Ctrl+O |
| Снимок экрана | F6 |
| Читать буфер обмена | Ctrl+V |
| Копировать текст | Ctrl+C |

## Язык и распознавание

При первом запуске используется язык Windows. Смена языка через меню не требует перезапуска и сохраняет результаты. 27 каталогов для 26 языков покрывают 209 из 257 территорий CLDR (81,3%); учитывается число территорий, а не население.

Язык → Как в системе

Зелёные квадраты обозначают позиции; выбранный результат становится красным. Изображения уменьшаются до 1920×1080, мелкие коды дополнительно ищутся в исходных пикселях. Фирменные радиальные коды могут не поддерживаться.

## Сборка, тесты и выпуски

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
