# QReader — Polski

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Język](../README.md#qreader)

Czytnik kodów QR dla Windows 7 i nowszych, korzystający ze standardowych kontrolek Windows. Odczytuje zrzuty ekranu, pliki graficzne i obrazy ze schowka. OpenCV wykrywa położenie, a ZXing odczytuje treść.

## Pobieranie i obsługa

[GitHub Releases](https://github.com/halfstay/qreader/releases)

Rozpakuj ZIP i uruchom qreader.exe; pozostaw libZXing.dll w tym samym folderze. Wskaż okno lub kontrolkę i kliknij albo przeciągnij, aby zaznaczyć obszar. Esc lub prawy przycisk anuluje. Zrzut jest kopiowany do schowka w oryginalnym rozmiarze.

| Plik | ⌨ |
| --- | --- |
| Otwórz obraz — BMP / JPG / PNG | Ctrl+O |
| Zrzut ekranu | F6 |
| Odczytaj schowek | Ctrl+V |
| Kopiuj tekst | Ctrl+C |

## Język i rozpoznawanie

Pierwsze uruchomienie używa języka Windows. Zmień język w menu bez ponownego uruchamiania i utraty wyników. 27 katalogów dla 26 języków obejmuje 209 z 257 terytoriów CLDR (81,3%); liczona jest liczba terytoriów, nie ludność.

Język → Zgodnie z systemem

Zielone kwadraty oznaczają położenie; wybrany wynik staje się czerwony. Obrazy są zmniejszane do 1920×1080, a małe kody dodatkowo wyszukiwane w oryginalnych pikselach. Własnościowe kody promieniste mogą być nieobsługiwane.

## Budowanie, testy i wydania

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
