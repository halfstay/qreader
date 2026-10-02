# QReader — 한국어

[![Build](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](../LICENSE)

[English](../README.md) · [언어](../README.md#qreader)

Windows 7 이상에서 실행되는 QR 코드 리더입니다. Windows 표준 컨트롤로 화면 캡처, 이미지 파일, 클립보드 이미지를 분석합니다. OpenCV가 위치를 찾고 ZXing이 내용을 해독합니다.

## 다운로드 및 사용

[GitHub Releases](https://github.com/halfstay/qreader/releases)

ZIP을 풀고 libZXing.dll과 같은 폴더의 qreader.exe를 실행하세요. 창이나 하위 컨트롤 위에서 클릭하거나 드래그하여 영역을 선택합니다. Esc 또는 마우스 오른쪽 버튼으로 취소합니다. 캡처한 이미지는 원본 크기로 클립보드에 복사됩니다.

| 파일 | ⌨ |
| --- | --- |
| 이미지 열기 — BMP / JPG / PNG | Ctrl+O |
| 화면 캡처 | F6 |
| 클립보드 읽기 | Ctrl+V |
| 텍스트 복사 | Ctrl+C |

## 언어 및 인식

처음 실행할 때 Windows 표시 언어를 따릅니다. 메뉴에서 재시작 없이 언어를 바꿀 수 있으며 결과와 선택 항목은 유지됩니다. 26개 언어, 27개 번역 리소스가 CLDR 국가·지역 257개 중 209개(81.3%)를 지원합니다. 인구 비율이 아닙니다.

언어 → 시스템 설정 따르기

녹색 정사각형이 위치를 표시하고 선택한 결과는 빨간색이 됩니다. 전체 이미지는 최대 1920×1080으로 축소하며 작은 코드는 원본 픽셀 영역에서도 찾습니다. 플랫폼 전용 방사형 코드는 지원되지 않을 수 있습니다.

## 빌드, 테스트 및 릴리스

[README (English)](../README.md#build) · [I18N](I18N.md) · [Tests](../TESTING.md) · [Contributing](../CONTRIBUTING.md)

MinGW-w64 · C++20 · CMake 3.10+ · OpenCV 4.5.5 · ZXing-C++ 1.4.0

[MIT License](../LICENSE) · [Third-party licenses](../THIRD_PARTY_NOTICES.md)
