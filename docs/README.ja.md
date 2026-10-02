# QReader — 日本語

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [言語](../README.md#qreader)

Windows 7 以降向けの QR コード読み取りツールです。標準の Windows コントロールを使い、画面キャプチャ、画像ファイル、クリップボードの画像を解析します。OpenCV で位置を検出し、ZXing で内容を読み取ります。

## ダウンロードと操作

[GitHub Releases](https://github.com/halfstay/qreader/releases)

ZIP を展開し、libZXing.dll と同じフォルダーにある qreader.exe を起動します。マウスをウィンドウや子コントロールに合わせてクリックするか、ドラッグで範囲を指定します。Esc または右クリックで中止できます。キャプチャは元のサイズでクリップボードにコピーされます。

| ファイル | ⌨ |
| --- | --- |
| 画像を開く — BMP / JPG / PNG | Ctrl+O |
| 画面をキャプチャ | F6 |
| クリップボードを読み取る | Ctrl+V |
| テキストをコピー | Ctrl+C |

## 言語と認識

初回は Windows の表示言語に従います。メニューから再起動せずに切り替えられ、結果と選択状態が保持されます。26 言語、27 種の翻訳を提供し、CLDR の国・地域数で 209/257（81.3%）をカバーします。人口比ではありません。

言語 → システムに合わせる

緑の正方形が位置を示し、選択した結果は赤になります。画像を最大 1920×1080 に縮小し、小さなコードは元の画素を使う領域でも検出します。独自形式の放射状コードには対応しない場合があります。

## ビルド・テスト・リリース

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
