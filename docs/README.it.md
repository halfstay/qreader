# QReader — Italiano

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Lingua](../README.md#qreader)

Lettore di codici QR per Windows 7 e successivi con controlli nativi di Windows. Analizza schermate, file immagine e immagini negli appunti. OpenCV individua i codici e ZXing ne decodifica il contenuto.

## Download e utilizzo

[GitHub Releases](https://github.com/halfstay/qreader/releases)

Estrai lo ZIP e avvia qreader.exe tenendo libZXing.dll nella stessa cartella. Punta a una finestra o a un controllo e fai clic, oppure trascina per selezionare un’area. Esc o il clic destro annulla. La cattura viene copiata negli appunti alle dimensioni originali.

| File | ⌨ |
| --- | --- |
| Apri immagine — BMP / JPG / PNG | Ctrl+O |
| Cattura schermo | F6 |
| Leggi appunti | Ctrl+V |
| Copia testo | Ctrl+C |

## Lingua e riconoscimento

Al primo avvio viene usata la lingua di Windows. Puoi cambiarla dal menu senza riavviare né perdere i risultati. I 27 cataloghi per 26 lingue coprono 209 dei 257 territori CLDR (81,3%), contando i territori e non la popolazione.

Lingua → Segui il sistema

I quadrati verdi indicano le posizioni; il risultato selezionato diventa rosso. Le immagini vengono ridotte a un massimo di 1920×1080; i codici piccoli sono cercati anche nei pixel originali. I codici radiali proprietari potrebbero non essere supportati.

## Compilazione, test e versioni

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
