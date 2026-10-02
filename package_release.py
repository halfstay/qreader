#!/usr/bin/env python3
"""Stage a Release build and create GitHub Release assets in a chosen directory."""
import argparse
import hashlib
from pathlib import Path
import re
import shutil
import subprocess
import tarfile
import zipfile

ROOT = Path(__file__).resolve().parent
VERSION = re.search(r"project\(QReader VERSION ([0-9]+\.[0-9]+\.[0-9]+)", (ROOT / "CMakeLists.txt").read_text()).group(1)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--version", action="store_true")
    parser.add_argument("--tag", help="Require an exact vX.Y.Z tag matching CMake")
    parser.add_argument("--build", type=Path, default=ROOT / ".build/mingw")
    parser.add_argument("--output", type=Path, default=ROOT / ".build/release")
    parser.add_argument("--cmake", default="cmake")
    args = parser.parse_args()
    if args.tag and args.tag != f"v{VERSION}":
        parser.error(f"tag {args.tag!r} must match v{VERSION}")
    if args.version:
        print(VERSION)
        return
    build, output = args.build.resolve(), args.output.resolve()
    cache = (build / "CMakeCache.txt").read_text()
    if "CMAKE_BUILD_TYPE:STRING=Release" not in cache:
        parser.error("package a Release build")
    output.mkdir(parents=True, exist_ok=True)
    stage = output / "stage" / f"qreader-{VERSION}-windows-x64"
    if stage.exists():
        shutil.rmtree(stage)
    stage.mkdir(parents=True)
    subprocess.run([args.cmake, f"-DCMAKE_INSTALL_PREFIX={stage}", "-P", "cmake_install.cmake"], cwd=build, check=True)
    subprocess.run(["python3", str(ROOT / "tests/check_imports.py"), "--require-stripped", str(stage / "qreader.exe"), str(stage / "libZXing.dll")], check=True)
    sources = []
    for pattern in ("*.cmake", "*.py", "*.md"):
        sources.extend(ROOT.glob(pattern))
    sources.extend(ROOT / name for name in ("CMakeLists.txt", "LICENSE", ".gitignore", ".clang-format"))
    for directory in ("src", "res", "tests", "docs", ".github"):
        sources.extend(p for p in (ROOT / directory).rglob("*") if p.is_file() and "__pycache__" not in p.parts and p.suffix != ".pyc")
    archive = output / f"qreader-{VERSION}-source.tar.gz"
    with tarfile.open(archive, "w:gz") as bundle:
        for path in sorted(set(sources)):
            bundle.add(path, arcname=f"qreader-{VERSION}/{path.relative_to(ROOT)}", recursive=False)
    (stage / "sources").mkdir(exist_ok=True)
    shutil.copy2(archive, stage / "sources" / archive.name)
    portable = output / f"qreader-{VERSION}-windows-x64.zip"
    with zipfile.ZipFile(portable, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as bundle:
        for path in sorted(stage.rglob("*")):
            if path.is_file():
                bundle.write(path, f"{stage.name}/{path.relative_to(stage)}")
    with zipfile.ZipFile(portable) as bundle:
        if bundle.testzip() is not None:
            raise RuntimeError("ZIP integrity check failed")
    checksums = "".join(f"{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}\n" for path in (portable, archive))
    (output / "SHA256SUMS.txt").write_text(checksums)
    print(f"Release {VERSION}: {portable}, {archive}, {output / 'SHA256SUMS.txt'}")


if __name__ == "__main__":
    main()
