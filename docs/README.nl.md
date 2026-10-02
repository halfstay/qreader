# QReader — Nederlands

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Taal](../README.md#qreader)

QR-codelezer voor Windows 7 en nieuwer met standaard Windows-besturingselementen. Leest schermafbeeldingen, afbeeldingsbestanden en afbeeldingen op het klembord. OpenCV vindt de codes en ZXing decodeert de inhoud.

## Downloaden en gebruiken

[GitHub Releases](https://github.com/halfstay/qreader/releases)

Pak de ZIP uit en start qreader.exe; houd libZXing.dll in dezelfde map. Wijs een venster of besturingselement aan en klik, of sleep om een gebied te kiezen. Esc of rechtsklikken annuleert. De opname wordt op originele grootte naar het klembord gekopieerd.

| Bestand | ⌨ |
| --- | --- |
| Afbeelding openen — BMP / JPG / PNG | Ctrl+O |
| Scherm vastleggen | F6 |
| Klembord lezen | Ctrl+V |
| Tekst kopiëren | Ctrl+C |

## Taal en herkenning

Bij de eerste start wordt de Windows-taal gevolgd. Wissel via het menu zonder opnieuw op te starten of resultaten te verliezen. De 27 catalogi voor 26 talen dekken 209 van 257 CLDR-gebieden (81,3%); het gaat om aantallen gebieden, niet inwoners.

Taal → Systeemtaal volgen

Groene vierkanten tonen de posities; de selectie wordt rood. Afbeeldingen worden verkleind tot maximaal 1920×1080; kleine codes worden ook in oorspronkelijke pixels gezocht. Gesloten radiale codeformaten worden mogelijk niet ondersteund.

## Bouwen, testen en uitgaven

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
