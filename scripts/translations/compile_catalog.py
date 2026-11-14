"""Compile source JSON catalogs into a platform-neutral resource library source."""
import argparse
import json
from pathlib import Path

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    catalogs = {}
    for path in sorted(args.source.rglob("*.json")):
        relative = path.relative_to(args.source)
        if len(relative.parts) < 2:
            raise ValueError(f"A domain directory is required: {path}")
        domain = relative.parts[0]
        source = json.loads(path.read_text(encoding="utf-8-sig"))
        target = catalogs.setdefault(domain, {"strings": {}, "invariants": []})
        for key, value in source["strings"].items():
            if not isinstance(value, str) or key in target["strings"]:
                raise ValueError(f"Invalid or duplicate translation: {domain}/{key}")
            target["strings"][key] = value
        target["invariants"].extend(source.get("invariants", []))
    if not catalogs:
        raise ValueError(f"No catalogs in {args.source}")
    payload = json.dumps({"catalogs": catalogs}, ensure_ascii=False, separators=(",", ":")).encode("utf-8")
    data = ",".join(str(byte) for byte in payload)
    code = f"""#include <cstddef>
#if defined(_WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif
namespace {{ constexpr unsigned char payload[] = {{{data}}}; }}
extern "C" EXPORT const unsigned char* RetroMediaLabCatalog(std::size_t* length) noexcept {{
    if (length) *length = sizeof(payload);
    return payload;
}}
"""
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(code, encoding="utf-8")

if __name__ == "__main__":
    main()
