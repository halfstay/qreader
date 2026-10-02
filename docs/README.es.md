# QReader — Español

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Idioma](../README.md#qreader)

Lector de códigos QR para Windows 7 y versiones posteriores. Reconoce capturas, archivos de imagen e imágenes del portapapeles mediante controles nativos de Windows. OpenCV localiza los códigos y ZXing los descodifica.

## Descarga y uso

[GitHub Releases](https://github.com/halfstay/qreader/releases)

Descarga el ZIP, extráelo y ejecuta qreader.exe junto a libZXing.dll. Mueve el ratón para seleccionar una ventana o un control; haz clic para capturarlo o arrastra para marcar un área. Esc o el botón derecho cancela. La captura se copia al portapapeles en su tamaño original.

| Archivo | ⌨ |
| --- | --- |
| Abrir imagen — BMP / JPG / PNG | Ctrl+O |
| Capturar pantalla | F6 |
| Leer portapapeles | Ctrl+V |
| Copiar texto | Ctrl+C |

## Idioma y reconocimiento

El idioma inicial sigue a Windows. Puedes cambiarlo desde el menú sin reiniciar ni perder los resultados. Hay 27 catálogos para 26 idiomas: 209 de 257 territorios de CLDR (81,3%), contados por territorio, no por población.

Idioma → Seguir el sistema

Los cuadrados verdes señalan las posiciones; el resultado seleccionado aparece en rojo. Las imágenes se ajustan a 1920×1080 y los códigos pequeños se vuelven a buscar en regiones con píxeles originales. Los códigos radiales propios de ciertas plataformas pueden no ser compatibles.

## Compilación, pruebas y versiones

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
