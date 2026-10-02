# QReader — Română

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Limbă](../README.md#qreader)

Cititor de coduri QR pentru Windows 7 și versiuni mai noi, cu controale Windows native. Citește capturi de ecran, fișiere imagine și imagini din clipboard. OpenCV localizează codurile, iar ZXing le decodează.

## Descărcare și utilizare

[GitHub Releases](https://github.com/halfstay/qreader/releases)

Extrage arhiva ZIP și rulează qreader.exe, păstrând libZXing.dll în același dosar. Indică o fereastră sau un control și fă clic, ori trage pentru a selecta o zonă. Esc sau clic dreapta anulează. Captura se copiază în clipboard la dimensiunea originală.

| Fișier | ⌨ |
| --- | --- |
| Deschide imagine — BMP / JPG / PNG | Ctrl+O |
| Capturează ecranul | F6 |
| Citește clipboardul | Ctrl+V |
| Copiază textul | Ctrl+C |

## Limbă și recunoaștere

La prima pornire se folosește limba Windows. O poți schimba din meniu fără repornire sau pierderea rezultatelor. Cele 27 de cataloage pentru 26 de limbi acoperă 209 din 257 de teritorii CLDR (81,3%); se numără teritoriile, nu populația.

Limbă → Urmează sistemul

Pătratele verzi indică pozițiile; selecția devine roșie. Imaginile se reduc la maximum 1920×1080; codurile mici sunt căutate și în zone cu pixeli originali. Codurile radiale proprietare pot să nu fie acceptate.

## Compilare, teste și versiuni

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
