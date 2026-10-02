# QReader — 简体中文

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [语言](../README.md#qreader)

二维码识别工具，适用于 Windows 7 及以上系统。支持窗口和子窗口截图、拖选区域、图片文件与剪贴板图片。使用原生 Windows 控件，先由 OpenCV 定位，再由 ZXing 解码。

## 下载与使用

[GitHub Releases](https://github.com/halfstay/qreader/releases)

下载 ZIP，解压后运行 qreader.exe；请将 libZXing.dll 与程序放在同一目录。截图时移动鼠标选择窗口或控件，单击确认，或拖动选择区域。Esc 或右键取消。完成的截图会以原尺寸复制到剪贴板。

| 文件 | ⌨ |
| --- | --- |
| 打开图片 — BMP / JPG / PNG | Ctrl+O |
| 屏幕截图 | F6 |
| 剪贴板识别 | Ctrl+V |
| 复制内容 | Ctrl+C |

## 语言与识别

首次运行跟随 Windows 界面语言。通过语言菜单即时切换，无需重启；识别结果与选中项会保留。支持 27 套界面资源、26 种语言；按 CLDR 的国家/地区数量统计，覆盖 209/257（81.3%），并非人口占比。

语言 → 跟随系统

黑底指示区以等大绿色正方形标记二维码；选择内容行后对应标记变红。整图按 1920×1080 上限缩小，大图中的小码使用原始像素局部区域补查。专有放射状平台码不属于标准二维码，可能无法识别。

## 构建、测试与发布

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
