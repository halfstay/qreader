# Third-party notices

QReader's original source code is licensed under the MIT License. Dependencies retain their own licenses; the MIT license does not replace them.

| Component | Version | License / source |
| --- | --- | --- |
| OpenCV | 4.5.5 | [Apache-2.0](https://github.com/opencv/opencv/tree/4.5.5) |
| ZXing-C++ | 1.4.0 | [Apache-2.0, with LGPL text codec components](https://github.com/zxing-cpp/zxing-cpp/tree/v1.4.0) |
| libjpeg-turbo | bundled with OpenCV | BSD-style / IJG / zlib, see installed license texts |
| libpng | bundled with OpenCV | libpng license |
| zlib | bundled with OpenCV | zlib license |
| Protobuf | bundled with OpenCV | BSD-3-Clause, dependency of OpenCV's objdetect / dnn build |
| SoftFloat / FDLIBM | included in OpenCV core | BSD-style / Sun permissive notices |
| MinGW-w64 / GCC runtime | build toolchain | MinGW-w64 notices / GPL with GCC Runtime Library Exception |

The install step copies dependency license texts into `licenses/`. In particular, ZXing 1.4.0 includes text codec code originating in Qt; its LGPL text, exception and upstream notices are included. This does not add Qt GUI controls or a dependency on Qt DLLs.

ZXing is built as a separate, replaceable `libZXing.dll`. The exact upstream source archive is included in `sources/zxing-1.4.0.tar.gz`. To rebuild it, configure this repository with the same MinGW-w64 ABI and run `cmake --build . --target ZXing`. Replace the DLL alongside `qreader.exe`. Modification and debugging of that library remain permitted under its license. Build scripts, archive hashes and flags are in `Dependencies.cmake` and `CMakeLists.txt`.

When distributing binaries built with another toolchain, retain its applicable runtime notices along with these files. Dependency versions are deliberately fixed for the CMake 3.10 / Windows 7 target; upgrades require rerunning the compatibility checks.

## Unicode CLDR data

The territory/language snapshot in `res/i18n/territories.csv` is derived from
Unicode CLDR 46. It is used only for the documented language-coverage statistic.
It is distributed under the Unicode license in `res/i18n/UNICODE-LICENSE.txt`
(and `licenses/UNICODE-LICENSE.txt` in binary packages). Source and checksum are
recorded in [I18N.md](docs/I18N.md).
