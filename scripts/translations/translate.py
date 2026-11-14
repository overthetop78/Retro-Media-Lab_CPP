"""Generate language catalogs from Constants using Argos Translate."""
import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CATALOGS = ROOT / "src/Translations/Catalogs"

def protect(text, words):
    saved = []
    for word in sorted(set(words), key=len, reverse=True):
        if not word:
            continue
        pattern = re.compile(r"(?<!\w)" + re.escape(word) + r"(?!\w)")
        def replace(match):
            token = f"ZXQ{len(saved)}QXZ"
            saved.append((token, match.group()))
            return token
        text = pattern.sub(replace, text)
    return text, saved

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--languages", nargs="+", default=["en-US", "fr-FR"])
    parser.add_argument("--catalogs", type=Path, default=CATALOGS)
    args = parser.parse_args()
    for language in args.languages:
        if not re.fullmatch(r"[A-Za-z0-9_-]+", language):
            raise ValueError(f"Invalid language: {language}")
        ignore_file = Path(__file__).parent / "ignores" / f"{language}.txt"
        ignored = [line.strip() for line in ignore_file.read_text(encoding="utf-8").splitlines()
                   if line.strip() and not line.lstrip().startswith("#")] if ignore_file.exists() else []
        translator = None
        if language != "en-US":
            import argostranslate.translate
            translator = argostranslate.translate.get_translation_from_codes("en", language.split("-")[0])
        for source in sorted((args.catalogs / "Constants").rglob("*.json")):
            base = json.loads(source.read_text(encoding="utf-8-sig"))
            invariants = set(base.get("invariants", []))
            invariant_values = [base["strings"][key] for key in invariants]
            result = {}
            for key, text in base["strings"].items():
                if key in invariants:
                    continue
                if translator is None:
                    result[key] = text
                    continue
                protected, saved = protect(text, ignored + invariant_values + re.findall(r"\{[^{}]+\}", text))
                translated = translator.translate(protected)
                for token, original in saved:
                    if translated.count(token) != 1:
                        raise ValueError(f"Argos changed invariant token for {language}: {key}")
                    translated = translated.replace(token, original)
                result[key] = translated
            destination = args.catalogs / "Languages" / language / source.relative_to(args.catalogs / "Constants")
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_text(json.dumps({"strings": result}, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
            print(destination)
if __name__ == "__main__":
    main()
