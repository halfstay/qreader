# QReader — Bahasa Indonesia

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Bahasa](../README.md#qreader)

Pembaca kode QR untuk Windows 7 dan versi lebih baru dengan kontrol Windows asli. Membaca tangkapan layar, berkas gambar, dan gambar papan klip. OpenCV mencari posisi kode, lalu ZXing membaca isinya.

## Unduh dan gunakan

[GitHub Releases](https://github.com/halfstay/qreader/releases)

Ekstrak ZIP dan jalankan qreader.exe; simpan libZXing.dll di folder yang sama. Arahkan penunjuk ke jendela atau kontrol lalu klik, atau seret untuk memilih area. Esc atau klik kanan membatalkan. Tangkapan disalin ke papan klip dalam ukuran asli.

| Berkas | ⌨ |
| --- | --- |
| Buka gambar — BMP / JPG / PNG | Ctrl+O |
| Tangkap layar | F6 |
| Baca papan klip | Ctrl+V |
| Salin teks | Ctrl+C |

## Bahasa dan pengenalan

Bahasa awal mengikuti Windows. Ganti melalui menu tanpa memulai ulang atau kehilangan hasil. Ada 27 katalog untuk 26 bahasa, mencakup 209 dari 257 wilayah CLDR (81,3%) menurut jumlah wilayah, bukan populasi.

Bahasa → Ikuti sistem

Kotak hijau menandai posisi; pilihan berubah merah. Gambar diperkecil hingga 1920×1080; kode kecil juga dicari pada area dengan piksel asli. Kode radial khusus platform mungkin tidak didukung.

## Kompilasi, pengujian, dan rilis

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
