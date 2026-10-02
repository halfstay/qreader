# QReader — Українська

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Мова](../README.md#qreader)

Програма читання QR-кодів для Windows 7 і новіших із типовими елементами керування Windows. Читає знімки екрана, файли зображень і зображення з буфера обміну. OpenCV знаходить коди, ZXing розшифровує вміст.

## Завантаження та використання

[GitHub Releases](https://github.com/halfstay/qreader/releases)

Розпакуйте ZIP і запустіть qreader.exe; libZXing.dll має бути в тій самій теці. Наведіть вказівник на вікно чи елемент і клацніть або перетягніть для вибору ділянки. Esc або права кнопка скасовує. Знімок копіюється в буфер обміну в початковому розмірі.

| Файл | ⌨ |
| --- | --- |
| Відкрити зображення — BMP / JPG / PNG | Ctrl+O |
| Знімок екрана | F6 |
| Читати буфер обміну | Ctrl+V |
| Копіювати текст | Ctrl+C |

## Мова та розпізнавання

Перший запуск використовує мову Windows. Змінюйте її через меню без перезапуску та втрати результатів. 27 каталогів для 26 мов охоплюють 209 із 257 територій CLDR (81,3%); це частка територій, а не населення.

Мова → Як у системі

Зелені квадрати позначають розташування; вибраний результат стає червоним. Зображення зменшуються до 1920×1080, дрібні коди додатково шукаються в початкових пікселях. Власні радіальні коди платформ можуть не підтримуватися.

## Збирання, перевірки та випуски

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
