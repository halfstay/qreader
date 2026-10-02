# QReader — Bahasa Melayu

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Bahasa](../README.md#qreader)

Pembaca kod QR untuk Windows 7 dan versi lebih baharu dengan kawalan Windows asli. Membaca tangkapan skrin, fail imej dan imej papan keratan. OpenCV mencari kedudukan kod dan ZXing menyahkod kandungannya.

## Muat turun dan guna

[GitHub Releases](https://github.com/halfstay/qreader/releases)

Ekstrak ZIP dan jalankan qreader.exe; letakkan libZXing.dll dalam folder yang sama. Halakan tetikus ke tetingkap atau kawalan lalu klik, atau seret untuk memilih kawasan. Esc atau klik kanan membatalkan. Tangkapan disalin ke papan keratan pada saiz asal.

| Fail | ⌨ |
| --- | --- |
| Buka imej — BMP / JPG / PNG | Ctrl+O |
| Tangkap skrin | F6 |
| Baca papan klip | Ctrl+V |
| Salin teks | Ctrl+C |

## Bahasa dan pengecaman

Bahasa awal mengikut Windows. Tukar melalui menu tanpa memulakan semula atau kehilangan hasil. Terdapat 27 katalog untuk 26 bahasa, meliputi 209 daripada 257 wilayah CLDR (81.3%) mengikut bilangan wilayah, bukan penduduk.

Bahasa → Ikut sistem

Petak hijau menandakan kedudukan; pilihan menjadi merah. Imej dikecilkan sehingga 1920×1080; kod kecil turut dicari dalam kawasan piksel asal. Kod jejari khas platform mungkin tidak disokong.

## Binaan, ujian dan keluaran

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
