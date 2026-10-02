#!/usr/bin/env python3
"""Check the distributed PE files, not the host compiler's feature macros."""
import re
import argparse
from pathlib import Path
import struct
import subprocess

ALLOWED_DLLS = {
    "kernel32.dll", "msvcrt.dll", "user32.dll", "gdi32.dll", "comctl32.dll",
    "comdlg32.dll", "shell32.dll", "dwmapi.dll", "advapi32.dll", "ole32.dll",
    "oleaut32.dll", "uuid.dll", "ws2_32.dll", "version.dll", "shlwapi.dll",
    "libzxing.dll",
}
NEW_APIS = {
    "GetDpiForWindow", "GetDpiForSystem", "SetProcessDpiAwarenessContext",
    "SetProcessDpiAwareness", "GetSystemTimePreciseAsFileTime", "WaitOnAddress",
    "WakeByAddressSingle", "WakeByAddressAll", "SetThreadDescription",
    "GetThreadDescription", "GetTempPath2W", "GetTempPath2A",
    "CreateFile2", "GetSystemCpuSetInformation", "VirtualAlloc2",
}

def check_stripped(path):
    data = Path(path).read_bytes()
    if data[:2] != b"MZ":
        raise RuntimeError(f"{path}: not a PE file")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe:pe + 4] != b"PE\0\0":
        raise RuntimeError(f"{path}: invalid PE signature")
    symbols, count = struct.unpack_from("<II", data, pe + 12)
    if symbols or count:
        raise RuntimeError(f"{path}: COFF symbol table was not stripped")
    sections = struct.unpack_from("<H", data, pe + 6)[0]
    optional_size = struct.unpack_from("<H", data, pe + 20)[0]
    table = pe + 24 + optional_size
    for index in range(sections):
        name = data[table + index * 40:table + index * 40 + 8].rstrip(b"\0")
        if name.startswith((b".debug", b".zdebug", b".stab")):
            raise RuntimeError(f"{path}: debug section remains: {name!r}")
    print(f"PASS {path}: stripped (no COFF symbols or debug sections)")

def check(path):
    text = subprocess.check_output(["x86_64-w64-mingw32-objdump", "-p", path], text=True)
    dlls = re.findall(r"DLL Name:\s+(\S+)", text)
    unknown = set(map(str.lower, dlls)) - ALLOWED_DLLS
    if unknown:
        raise RuntimeError(f"{path}: unexpected runtime dependencies: {sorted(unknown)}")
    major = int(re.search(r"MajorSubsystemVersion\s+(\d+)", text).group(1))
    minor = int(re.search(r"MinorSubsystemVersion\s+(\d+)", text).group(1))
    if (major, minor) > (6, 1):
        raise RuntimeError(f"{path}: subsystem {major}.{minor} is newer than Windows 7")
    names = set(re.findall(r"^\s+[0-9a-f]+\s+\d+\s+(\w+)\s*$", text, re.MULTILINE))
    if names & NEW_APIS:
        raise RuntimeError(f"{path}: post-Win7 static imports: {sorted(names & NEW_APIS)}")
    print(f"PASS {path}: subsystem {major}.{minor}; " + ", ".join(dlls))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--require-stripped", action="store_true", help="Also reject unstripped Release binaries")
    parser.add_argument("files", nargs="+")
    args = parser.parse_args()
    for path in args.files:
        check(path)
        if args.require_stripped:
            check_stripped(path)
