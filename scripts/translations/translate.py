"""Generate en-US from Constants, then translate en-US catalogs with Argos."""
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

def translate_preserving_invariants(translator, text, words):
    protected, saved = protect(text, words)
    translated = translator.translate(protected)
    if all(translated.count(token) == 1 for token, _ in saved):
        for token, original in saved:
            translated = translated.replace(token, original)
        return translated
    # Some models alter synthetic tokens. Translate only the text between invariants
    # in that case; invariant bytes never pass through the translator.
    originals = dict(saved)
    parts = re.split("(" + "|".join(re.escape(token) for token in originals) + ")", protected)
    result = []
    for part in parts:
        if part in originals:
            result.append(originals[part])
        elif part.strip():
            leading = part[:len(part) - len(part.lstrip())]
            trailing = part[len(part.rstrip()):]
            result.append(leading + translator.translate(part.strip()) + trailing)
        else:
            result.append(part)
    return "".join(result)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--languages", nargs="+", default=["en-US", "fr-FR"])
    parser.add_argument("--all", action="store_true", help="Generate all languages from Languages.json")
    parser.add_argument("--catalogs", type=Path, default=CATALOGS)
    parser.add_argument("--files", nargs="+", help="Paths relative to Constants, for example App/menus.json")
    args = parser.parse_args()
    language_metadata = json.loads((CATALOGS.parent / "Languages.json").read_text(encoding="utf-8-sig"))
    argos_codes = {language["code"]: language["argos"] for language in language_metadata}
    if args.all:
        args.languages = list(argos_codes)
    for language in args.languages:
        if not re.fullmatch(r"[A-Za-z0-9_-]+", language):
            raise ValueError(f"Invalid language: {language}")

    constants = args.catalogs / "Constants"
    available = {path.relative_to(constants).as_posix(): path for path in sorted(constants.rglob("*.json"))}
    selected = args.files or list(available)
    missing = set(selected) - available.keys()
    if missing:
        raise ValueError(f"Unknown catalog files: {', '.join(sorted(missing))}")
    sources = []
    for relative in dict.fromkeys(selected):
        base = json.loads(available[relative].read_text(encoding="utf-8-sig"))
        invariants = set(base.get("invariants", []))
        english = {key: value for key, value in base["strings"].items() if key not in invariants}
        sources.append((relative, english, [base["strings"][key] for key in invariants]))

    def write_catalog(language, relative, strings):
        destination = args.catalogs / "Languages" / language / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text(json.dumps({"strings": strings}, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(destination)

    # en-US is always prepared first, including when only a target language was requested.
    for relative, english, _ in sources:
        write_catalog("en-US", relative, english)

    ignore_file = Path(__file__).parent / "ignores" / "common.txt"
    ignored = [line.strip() for line in ignore_file.read_text(encoding="utf-8").splitlines()
               if line.strip() and not line.lstrip().startswith("#")]
    unavailable = []
    for language in dict.fromkeys(args.languages):
        if language == "en-US":
            continue
        import argostranslate.translate
        target_code = argos_codes.get(language, language.split("-")[0])
        try:
            translator = argostranslate.translate.get_translation_from_codes("en", target_code)
            if translator is None:
                raise ValueError("No translation")
        except (AttributeError, StopIteration, ValueError):
            unavailable.append(f"{language} (en -> {target_code})")
            print(f"Argos translation unavailable: {unavailable[-1]}", flush=True)
            continue
        for relative, _, invariant_values in sources:
            english_path = args.catalogs / "Languages" / "en-US" / relative
            english = json.loads(english_path.read_text(encoding="utf-8-sig"))["strings"]
            result = {}
            for key, text in english.items():
                result[key] = translate_preserving_invariants(translator, text,
                    ignored + invariant_values + re.findall(r"\{[^{}]+\}", text))
            write_catalog(language, relative, result)
    if unavailable:
        raise RuntimeError("Argos translation unavailable: " + ", ".join(unavailable))
if __name__ == "__main__":
    main()
