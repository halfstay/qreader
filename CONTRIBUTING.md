# Contributing

Issues, pull requests and translation corrections are welcome. Bug reports should include the Windows version, display scale, monitor arrangement, reproduction steps and a sample you can share. Remove private information from screenshots before publishing them.

- Keep MinGW-w64, C++20, CMake 3.10 and Windows 7 compatibility.
- Use Win32 standard controls and GDI; OpenCV locates QR codes and ZXing decodes them.
- Put code in `src/`, resources in `res/`, and tests in `tests/`. Choose any build directory; `.build/` is the convention used in examples, not a requirement. Keep generated output out of Git.
- Format C++ sources and headers with clang-format 14 using the root `.clang-format`: `clang-format -i src/*.cpp src/*.h tests/*.cpp res/resource.h`. Use `--dry-run --Werror` instead of `-i` to check formatting.
- Add a regression test for behavioral fixes. Run CTest, PE import/strip checks and `python3 tests/check_i18n.py`; perform the relevant checks in [TESTING.md](TESTING.md).
- Follow [I18N.md](docs/I18N.md) for translations. Preserve format fields, test immediate language switching and check long labels and right-to-left layouts. Language coverage is not a substitute for native-speaker review.
- Do not commit build output, downloaded dependencies, personal settings or private screenshots.
- Do not introduce imports that require newer Windows versions. Dynamically query optional APIs and retain a Windows 7 fallback.
- A release tag must match the CMake version and the executable version resources. Pushing `vX.Y.Z` runs CI and publishes the release after successful validation; see [README.md](README.md#automatic-github-releases).

Describe the user-visible change, validation and any untested environments in a pull request. Contributions to QReader are provided under its MIT license; third-party code/data retain their original licenses and need attribution.
