# QReader — Deutsch

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Sprache](../README.md#qreader)

QR-Code-Leser für Windows 7 und neuer mit nativen Windows-Steuerelementen. Liest Bildschirmaufnahmen, Bilddateien und Bilder aus der Zwischenablage. OpenCV findet die Positionen, ZXing decodiert den Inhalt.

## Download und Bedienung

[GitHub Releases](https://github.com/halfstay/qreader/releases)

ZIP herunterladen, entpacken und qreader.exe starten; libZXing.dll muss im selben Ordner liegen. Mit der Maus ein Fenster oder Steuerelement auswählen und klicken, oder einen Bereich aufziehen. Esc oder Rechtsklick bricht ab. Aufnahmen werden in Originalgröße in die Zwischenablage kopiert.

| Datei | ⌨ |
| --- | --- |
| Bild öffnen — BMP / JPG / PNG | Ctrl+O |
| Bildschirm aufnehmen | F6 |
| Zwischenablage lesen | Ctrl+V |
| Text kopieren | Ctrl+C |

## Sprache und Erkennung

Beim ersten Start gilt die Windows-Anzeigesprache. Ein Sprachwechsel im Menü benötigt keinen Neustart und erhält die Ergebnisse. 27 Kataloge für 26 Sprachen decken 209 von 257 CLDR-Gebieten ab (81,3%); gezählt werden Gebiete, nicht Einwohner.

Sprache → Systemsprache verwenden

Grüne Quadrate zeigen die Positionen, die Auswahl wird rot. Bilder werden auf höchstens 1920×1080 verkleinert; kleine Codes werden zusätzlich in Bereichen mit Originalpixeln gesucht. Proprietäre radiale Codes werden möglicherweise nicht unterstützt.

## Bauen, testen und veröffentlichen

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
