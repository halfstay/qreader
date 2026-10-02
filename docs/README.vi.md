# QReader — Tiếng Việt

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [Ngôn ngữ](../README.md#qreader)

Trình đọc mã QR cho Windows 7 trở lên, sử dụng các điều khiển gốc của Windows. Phân tích ảnh chụp màn hình, tệp ảnh và ảnh trong bộ nhớ tạm. OpenCV xác định vị trí, ZXing giải mã nội dung.

## Tải xuống và sử dụng

[GitHub Releases](https://github.com/halfstay/qreader/releases)

Giải nén ZIP rồi chạy qreader.exe; giữ libZXing.dll trong cùng thư mục. Trỏ chuột vào cửa sổ hoặc điều khiển rồi nhấp, hoặc kéo để chọn vùng. Esc hoặc nhấp chuột phải để hủy. Ảnh chụp được sao chép vào bộ nhớ tạm với kích thước gốc.

| Tệp | ⌨ |
| --- | --- |
| Mở ảnh — BMP / JPG / PNG | Ctrl+O |
| Chụp màn hình | F6 |
| Đọc bộ nhớ tạm | Ctrl+V |
| Sao chép văn bản | Ctrl+C |

## Ngôn ngữ và nhận dạng

Lần đầu chạy sử dụng ngôn ngữ Windows. Đổi ngôn ngữ qua menu mà không khởi động lại hoặc mất kết quả. Có 27 bộ dịch cho 26 ngôn ngữ, bao phủ 209/257 vùng CLDR (81,3%) theo số vùng, không theo dân số.

Ngôn ngữ → Theo hệ thống

Hình vuông xanh đánh dấu vị trí; mục được chọn chuyển đỏ. Ảnh được thu nhỏ tối đa 1920×1080; mã nhỏ còn được tìm trong vùng điểm ảnh gốc. Mã dạng tia độc quyền của một số nền tảng có thể không được hỗ trợ.

## Biên dịch, kiểm thử và phát hành

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
