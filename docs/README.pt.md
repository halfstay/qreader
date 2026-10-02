# QReader — Português

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Idioma](../README.md#qreader)

Leitor de códigos QR para Windows 7 ou superior, com controles nativos do Windows. Analisa capturas, arquivos de imagem e imagens da área de transferência. OpenCV localiza os códigos e ZXing decodifica o conteúdo.

## Download e uso

[GitHub Releases](https://github.com/halfstay/qreader/releases)

Baixe e extraia o ZIP; execute qreader.exe mantendo libZXing.dll na mesma pasta. Aponte para uma janela ou controle e clique para capturar, ou arraste para selecionar uma área. Esc ou o botão direito cancela. A captura é copiada no tamanho original para a área de transferência.

| Arquivo | ⌨ |
| --- | --- |
| Abrir imagem — BMP / JPG / PNG | Ctrl+O |
| Capturar tela | F6 |
| Ler área de transferência | Ctrl+V |
| Copiar texto | Ctrl+C |

## Idioma e reconhecimento

O idioma inicial acompanha o Windows. Troque pelo menu sem reiniciar nem perder os resultados. São 27 catálogos para 26 idiomas, cobrindo 209 de 257 territórios CLDR (81,3%), por número de territórios, não por população.

Idioma → Seguir o sistema

Quadrados verdes marcam as posições; a seleção fica vermelha. Imagens são reduzidas até 1920×1080, com busca adicional de códigos pequenos nos pixels originais. Códigos radiais proprietários podem não ser reconhecidos.

## Compilação, testes e versões

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
