#!/usr/bin/env python3
"""Check complete catalogs, format fields and the CLDR territory-count target."""
import csv
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
LOCALES = ROOT / "res" / "i18n"


def catalog(tag):
    result = {}
    for line in (LOCALES / f"{tag}.lang").read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("#"):
            continue
        key, value = line.split("=", 1)
        assert key not in result and value.strip(), f"{tag}: duplicate/empty {key}"
        assert not re.search(r"\\(?![nt\\])", value), f"{tag}: bad escape in {key}"
        result[key] = value
    return result


def check():
    rows = [line.split("|") for line in (LOCALES / "languages.txt").read_text(encoding="utf-8").splitlines() if line and not line.startswith("#")]
    tags = [row[0] for row in rows]
    assert len(tags) == len(set(tags))
    english = catalog("en")
    for row in rows:
        assert len(row) == 5 and row[4] in ("ltr", "rtl") and all(row)
        tag = row[0]
        guide = "README.md" if tag == "en" else f"docs/README.{tag}.md"
        assert (ROOT / guide).is_file(), f"{tag}: missing language guide"
        assert f"]({guide})" in (ROOT / "README.md").read_text(encoding="utf-8"), f"{tag}: missing guide link"
        translation = catalog(tag)
        assert translation.keys() == english.keys(), f"{tag}: incomplete catalog"
        for key, value in translation.items():
            assert sorted(re.findall(r"\{[^{}]+\}", value)) == sorted(re.findall(r"\{[^{}]+\}", english[key])), f"{tag}: placeholders in {key}"
    assert {p.stem for p in LOCALES.glob("*.lang")} == set(tags)
    # Script variants are not interchangeable. Both supported Chinese scripts
    # are required before generic CLDR 'zh' contributes to the coverage count.
    supported = set(tags) - {"zh-CN", "zh-TW"}
    if {"zh-CN", "zh-TW"} <= set(tags):
        supported.update(("zh", "zh_Hans", "zh_Hant"))
    regions = list(csv.DictReader((LOCALES / "territories.csv").open(encoding="utf-8")))
    assert len(regions) == 257 and len({r["territory"] for r in regions}) == 257
    covered = [r for r in regions if set(r["official_languages"].split()) & supported]
    assert len(covered) / len(regions) >= .8, "Territory coverage below 80%"
    print(f"PASS {len(tags)} complete catalogs × {len(english)} keys; {len(covered)}/{len(regions)} territories ({len(covered)/len(regions):.1%})")
    return tags, regions, covered


if __name__ == "__main__":
    check()
