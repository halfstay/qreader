# QReader — 繁體中文

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [語言](../README.md#qreader)

適用於 Windows 7 及更新版本的 QR 碼閱讀工具。支援視窗與子視窗擷取、拖曳選取區域、圖片檔案及剪貼簿圖片。使用原生 Windows 控制項，由 OpenCV 定位，再由 ZXing 解碼。

## 下載與使用

[GitHub Releases](https://github.com/halfstay/qreader/releases)

下載 ZIP 並解壓縮，執行 qreader.exe；請將 libZXing.dll 放在同一目錄。移動滑鼠選取視窗或控制項，按一下確認，或拖曳選取範圍。Esc 或按右鍵取消。完成的擷取會以原始尺寸複製到剪貼簿。

| 檔案 | ⌨ |
| --- | --- |
| 開啟圖片 — BMP / JPG / PNG | Ctrl+O |
| 螢幕擷取 | F6 |
| 辨識剪貼簿 | Ctrl+V |
| 複製內容 | Ctrl+C |

## 語言與辨識

首次執行依照 Windows 介面語言。語言選單可立即切換，不需重新啟動，辨識結果與選取項目都會保留。提供 27 套介面資源、26 種語言；依 CLDR 國家／地區數量計算涵蓋 209/257（81.3%），並非人口比例。

語言 → 跟隨系統

黑底位置區使用等大的綠色正方形，選取內容後對應標記轉紅。整張圖片縮至 1920×1080 以內，大圖的小碼另以原始像素區域檢查。專有放射狀平台碼可能無法辨識。

## 建置、測試與發布

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
