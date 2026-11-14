"""Embed translated startup errors without depending on Qt or language DLLs."""
import argparse
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--catalogs", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    base = json.loads((args.catalogs / "Constants/App/errors.json").read_text(encoding="utf-8"))["strings"]
    rows = []
    for directory in sorted((args.catalogs / "Languages").iterdir()):
        if not directory.is_dir():
            continue
        localized = json.loads((directory / "App/errors.json").read_text(encoding="utf-8"))["strings"]
        if localized.keys() != base.keys():
            raise ValueError(f"Incomplete errors: {directory.name}")
        for key, text in localized.items():
            rows.append("    {" + ", ".join(json.dumps(s, ensure_ascii=True) for s in (directory.name, key, text)) + "},")
    code = """#pragma once
#include <string_view>
namespace RetroMediaLab::ErrorCatalog {
struct Entry { std::string_view language, key, text; };
inline constexpr Entry Entries[] = {
""" + "\n".join(rows) + "\n};\n}\n"
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(code, encoding="utf-8")


if __name__ == "__main__":
    main()
