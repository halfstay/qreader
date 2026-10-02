# QReader — Français

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Langue](../README.md#qreader)

Lecteur de codes QR pour Windows 7 et versions ultérieures, avec contrôles Windows natifs. Analyse les captures, les fichiers image et le presse-papiers. OpenCV repère les codes et ZXing les décode.

## Téléchargement et utilisation

[GitHub Releases](https://github.com/halfstay/qreader/releases)

Téléchargez et décompressez le ZIP, puis lancez qreader.exe avec libZXing.dll dans le même dossier. Survolez une fenêtre ou un contrôle et cliquez pour le capturer, ou faites glisser pour choisir une zone. Échap ou le clic droit annule. La capture est copiée à sa taille originale dans le presse-papiers.

| Fichier | ⌨ |
| --- | --- |
| Ouvrir une image — BMP / JPG / PNG | Ctrl+O |
| Capturer l’écran | F6 |
| Lire le presse-papiers | Ctrl+V |
| Copier le texte | Ctrl+C |

## Langue et reconnaissance

La langue suit Windows au premier lancement. Le menu permet de la changer sans redémarrage ni perte des résultats. Les 27 catalogues couvrent 26 langues et 209 des 257 territoires CLDR (81,3%), selon le nombre de territoires, pas la population.

Langue → Suivre le système

Les carrés verts indiquent les positions ; le résultat sélectionné devient rouge. Les images sont réduites à 1920×1080 au maximum ; les petits codes sont aussi recherchés dans les pixels originaux. Les codes radiaux propriétaires peuvent ne pas être reconnus.

## Compilation, tests et versions

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
