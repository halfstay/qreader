# QReader — Türkçe

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Dil](../README.md#qreader)

Windows 7 ve sonrası için yerel Windows denetimleri kullanan QR kod okuyucu. Ekran görüntülerini, resim dosyalarını ve pano resimlerini okur. OpenCV konumları bulur, ZXing içeriği çözer.

## İndirme ve kullanım

[GitHub Releases](https://github.com/halfstay/qreader/releases)

ZIP dosyasını açıp qreader.exe dosyasını çalıştırın; libZXing.dll aynı klasörde olmalıdır. Bir pencereye veya denetime gelip tıklayın ya da alan seçmek için sürükleyin. Esc veya sağ tıklama iptal eder. Görüntü özgün boyutuyla panoya kopyalanır.

| Dosya | ⌨ |
| --- | --- |
| Görüntü aç — BMP / JPG / PNG | Ctrl+O |
| Ekranı yakala | F6 |
| Panoyu oku | Ctrl+V |
| Metni kopyala | Ctrl+C |

## Dil ve tanıma

İlk çalıştırmada Windows dili izlenir. Menüden yeniden başlatmadan ve sonuçları kaybetmeden dil değiştirebilirsiniz. 26 dil için 27 katalog, CLDR bölgelerinin 209/257’sini (%81,3) kapsar; bu oran nüfusa değil bölge sayısına göredir.

Dil → Sistem dilini kullan

Yeşil kareler konumları gösterir; seçilen sonuç kırmızı olur. Resimler en fazla 1920×1080 boyutuna küçültülür, küçük kodlar özgün pikselli alanlarda da aranır. Platformlara özel ışınsal kodlar desteklenmeyebilir.

## Derleme, test ve sürümler

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
