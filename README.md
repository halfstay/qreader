# QReader

[![Build and test](https://github.com/halfstay/qreader/actions/workflows/build.yml/badge.svg)](https://github.com/halfstay/qreader/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/halfstay/qreader)](https://github.com/halfstay/qreader/releases)
[![Downloads](https://img.shields.io/github/downloads/halfstay/qreader/total)](https://github.com/halfstay/qreader/releases)
[![MIT](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Windows](https://img.shields.io/badge/Windows-7%2B-0078D6?logo=windows)](#requirements)
[![Native UI](https://img.shields.io/badge/GUI-Win32-blue)](#features)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus)](https://isocpp.org/)
[![MinGW-w64](https://img.shields.io/badge/compiler-MinGW--w64-blue)](https://www.mingw-w64.org/)
[![CMake](https://img.shields.io/badge/CMake-3.10%2B-064F8C?logo=cmake)](CMakeLists.txt)
[![OpenCV](https://img.shields.io/badge/OpenCV-4.5.5-5C3EE8?logo=opencv)](Dependencies.cmake)
[![ZXing](https://img.shields.io/badge/ZXing--C%2B%2B-1.4.0-orange)](Dependencies.cmake)
[![UI locales](https://img.shields.io/badge/UI_locales-27-success)](docs/I18N.md)
[![Territory coverage](https://img.shields.io/badge/territories-209%2F257_%2881.3%25%29-success)](docs/I18N.md#coverage)
[![Strip](https://img.shields.io/badge/Release-automatically_stripped-success)](CMakeLists.txt)
[![Stars](https://img.shields.io/github/stars/halfstay/qreader?style=flat)](https://github.com/halfstay/qreader/stargazers)
[![Forks](https://img.shields.io/github/forks/halfstay/qreader?style=flat)](https://github.com/halfstay/qreader/forks)
[![Issues](https://img.shields.io/github/issues/halfstay/qreader)](https://github.com/halfstay/qreader/issues)
[![Pull requests](https://img.shields.io/github/issues-pr/halfstay/qreader)](https://github.com/halfstay/qreader/pulls)
[![Last commit](https://img.shields.io/github/last-commit/halfstay/qreader)](https://github.com/halfstay/qreader/commits)

A native Windows QR code reader for screenshots, image files and the clipboard. OpenCV locates codes; ZXing-C++ decodes their content. No Qt, GTK, browser UI or online recognition service.

<!-- LANGUAGES -->
[English](README.md) · [简体中文](docs/README.zh-CN.md) · [繁體中文](docs/README.zh-TW.md) · [Español](docs/README.es.md) · [Français](docs/README.fr.md) · [Deutsch](docs/README.de.md) · [Português](docs/README.pt.md) · [Русский](docs/README.ru.md) · [العربية](docs/README.ar.md) · [日本語](docs/README.ja.md) · [한국어](docs/README.ko.md) · [हिन्दी](docs/README.hi.md) · [বাংলা](docs/README.bn.md) · [اردو](docs/README.ur.md) · [Bahasa Indonesia](docs/README.id.md) · [Bahasa Melayu](docs/README.ms.md) · [Tiếng Việt](docs/README.vi.md) · [ไทย](docs/README.th.md) · [Türkçe](docs/README.tr.md) · [Italiano](docs/README.it.md) · [فارسی](docs/README.fa.md) · [Kiswahili](docs/README.sw.md) · [Nederlands](docs/README.nl.md) · [Polski](docs/README.pl.md) · [Українська](docs/README.uk.md) · [Română](docs/README.ro.md) · [Ελληνικά](docs/README.el.md)
<!-- /LANGUAGES -->

## Features

- Point at a window or nested control to highlight it in green. Click to capture it, or drag in any direction to select an area. Esc or right-click cancels.
- Completed captures are copied to the clipboard at their original size. Cancelling preserves the clipboard. If copying fails, recognition still continues.
- Open or drop BMP, JPG/JPEG and PNG files, including Unicode paths. Read clipboard PNG, DIB, DIBV5, bitmap or one copied image file.
- Whole-image analysis fits within 1920 × 1080 without enlarging small images. Overlapping original-pixel regions and compact QR proposals recover small codes in large windows and crowded rows. Every candidate still passes OpenCV localization before ZXing decoding.
- Black position panel with equal-size square markers, sized from the shortest detected edge. Select a result to turn its marker red and view or copy the full text. Duplicate content at different positions stays separate.
- Background recognition, repeatable detection and high-DPI layout. Positions keep their spatial meaning when the UI uses a right-to-left language.
- 27 embedded UI catalogs, covering 26 languages. First launch follows Windows UI language preferences. Change **Language** at any time; no restart or extra language download is required. Text lengths, fonts and layout direction update together.
- The About dialog includes a clickable project URL. Decoded QR content is never automatically opened or executed.

## Requirements

Windows 7 SP1 or later, x64 for the provided binary package. Keep `qreader.exe` and `libZXing.dll` together. OpenCV and MinGW runtimes are linked statically; ZXing remains a replaceable DLL. No OpenCV installation is needed.

This project targets Windows 7 APIs and checks executable imports. Automated GUI testing runs under Wine; actual Windows 7 hardware, mixed-DPI displays and all language font combinations still require the checks in [TESTING.md](TESTING.md). Windows-owned file-picker controls depend on installed OS language packs; QReader's own text and controls switch immediately.

## Use

Download the Windows ZIP from [Releases](https://github.com/halfstay/qreader/releases), extract it and run `qreader.exe`. Optional: run `qreader.exe "C:\Pictures\codes.png"`.

| Action | Shortcut or control |
| --- | --- |
| Open image | Open image / Ctrl+O / drag one file onto the window |
| Capture screen | Capture screen / F6 |
| Read clipboard image | Read clipboard / Ctrl+V |
| Capture window or child control | Hover, then click |
| Capture an area | Drag, then release |
| Cancel capture | Esc / right-click |
| Copy decoded text | Select a result, then Copy text / Ctrl+C |
| Change language | Language → a language, or Follow system |
| Open project page | Help → About QReader → GitHub link |

Language choice is saved per user at `HKCU\Software\QReader\Language`. Choose **Follow system** to restore automatic selection. Unknown languages fall back to English; regional variants match a supported language, with separate Simplified and Traditional Chinese catalogs.

Capture uses a frozen image of the visible desktop. It cannot recover pixels hidden behind another window. Green selection borders are excluded from the copied image. The gray frame in the position panel represents the analyzed image boundary.

## Recognition limits

Small, dense codes can require the original screen pixels: whole-desktop resizing may erase modules. QReader additionally scans overlapping 512 × 512 regions and compact candidates to approximate a careful manual crop. This can take longer on large or busy screenshots.

A seven-code row inside a browser screenshot is covered by the current recovery path. Decorative platform codes without the standard QR structure, such as radial proprietary codes, are not generally supported. A located but undecodable candidate remains visible; localization is not proof of a valid payload.

## Build

Use MinGW-w64 with C++20 support and an MSVCRT/Windows 7-compatible runtime. CMake **3.10+** is supported. Dependency archives are pinned and SHA-256 checked: OpenCV 4.5.5 and ZXing-C++ 1.4.0. Choose any build directory, including one outside the checkout; `.build/` is only the convention used in these examples. Dependencies default to `<build>/dependencies/` and license copies to `<build>/licenses/`. Set `-DQREADER_DEPENDENCY_DIR=/path/to/cache` to reuse a dependency cache.

On Windows, from a MinGW-w64 toolchain shell:

```sh
mkdir -p .build/release
cd .build/release
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ../..
cmake --build . -- -j4
cmake -DCMAKE_INSTALL_PREFIX=../dist -P cmake_install.cmake
ctest --output-on-failure
```

Cross-build and test on Debian/Ubuntu:

```sh
sudo apt-get install g++-mingw-w64-x86-64-posix cmake make python3 wine wine64 xvfb xauth
mkdir -p .build/mingw
cd .build/mingw
cmake -DCMAKE_TOOLCHAIN_FILE=../../mingw-w64.cmake \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_CROSSCOMPILING_EMULATOR=wine ../..
cmake --build . -- -j4
WINEPREFIX="$PWD/../wine" WINEDEBUG=-all xvfb-run -a \
  -s '-screen 0 2560x1600x24' ctest --output-on-failure
cd ../..
python3 tests/check_i18n.py
python3 package_release.py
```

Release linking automatically uses `--strip-all` for every generated EXE/DLL, including ZXing and test programs. Exports, icons and version resources are retained. Debug and RelWithDebInfo retain debugging symbols. The package script checks stripped PE imports and creates ZIP/source/checksum assets in `.build/release/`.

For another build or release directory, run `python3 package_release.py --build /path/to/build --output /path/to/releases`. Both paths are unrestricted; `.build/mingw` and `.build/release` are just defaults.

## Automatic GitHub Releases

The workflow builds and tests every push/PR. A pushed version tag automatically publishes a GitHub Release after all checks pass:

```sh
# The tag must match project(QReader VERSION ...) in CMakeLists.txt.
git tag -a v1.2.0 -m "QReader 1.2.0"
git push origin v1.2.0
```

No manual workflow run is needed. Assets are `qreader-1.2.0-windows-x64.zip`, `qreader-1.2.0-source.tar.gz` and `SHA256SUMS.txt`. The ZIP includes documentation, licenses, the ZXing source archive and QReader source. A mismatched tag fails validation; retrying the same tag updates its assets. Publishing uses the repository's `GITHUB_TOKEN` with `contents: write` only in the release job. Repository/organization policy must allow that permission.

## Internationalization and contributing

See [I18N.md](docs/I18N.md) for the supported locales, territory-count methodology, translation format and maintenance checks. The 81.3% figure means at least one CLDR official or de facto official language has a complete catalog in **209 of 257 listed territories**. It is not a population percentage; English fallback is not counted as translated support.

Contributions, translations and reproducible bug reports are welcome: [CONTRIBUTING.md](CONTRIBUTING.md). Include the Windows version, display scale and a shareable sample. Keep private screenshots out of source archives.

## License and layout

QReader is [MIT licensed](LICENSE). Dependencies and Unicode data retain their own licenses; see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

```text
src/                C++ application, recognition and localization
res/                icon, Win32 resources, UTF-8 translations, coverage data
tests/              recognition, clipboard, GUI and localization checks
docs/               translated README guides and internationalization documentation
.build/             example build location (any directory is supported)
CMakeLists.txt      build entry point
Localization.cmake  embed language resources without extra build tools
package_release.py  release packaging
```

The editable icon is `res/qreader.svg`; `res/qreader.ico` contains 16–256 px images. It is embedded in the EXE and used in the window and About dialog.
