"""Generate filesystem-name fallback data from the canonical translation catalog."""
import argparse
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    values = json.loads(args.source.read_text(encoding='utf-8-sig'))['strings']
    if not values or any(not isinstance(k, str) or not isinstance(v, str) for k, v in values.items()):
        raise ValueError('Invalid filesystem-name catalog')
    rows = ['    {' + json.dumps(k, ensure_ascii=True) + ', ' + json.dumps(v, ensure_ascii=True) + '},' for k, v in sorted(values.items())]
    code = '#pragma once\n#include <string_view>\nnamespace RetroMediaLab::MediaFileSystems::NameData {\nstruct Entry {std::string_view key,text;};\ninline constexpr Entry Entries[]{\n' + '\n'.join(rows) + '\n};\n}\n'
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(code, encoding='utf-8')


if __name__ == '__main__':
    main()
