"""Audit application literals and error catalogs; no external files or media required."""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TOKEN = re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|(?:L|u8)?"(?:\\.|[^"\\])*"|(?:L)?\x27(?:\\.|[^\x27\\])*\x27|(?<![\w.])(?:0x[\da-fA-F]+|\d+\.\d+|\.\d+|\d+)(?![\w.])')
CONSTANT_HEADERS = {
    "src/Common/Constants/AppConstants.h", "src/App/Themes/RetroMediaLab/ThemeConstants.h",
    "src/Translations/include/RetroMediaLab/Translations/TranslationConstants.h",
    "src/Launcher/LauncherConstants.h", "src/Common/PlatformConstants.h",
    "src/Common/DiagnosticConstants.h", "src/Common/ErrorKeys.h",
    "src/Infrastructure/Engines/EngineConstants.h",
    "src/Infrastructure/Maintenance/GwToolsConstants.h",
    "src/Common/Constants/DisplayConstants.h",
    "src/Common/ProcessConstants.h", "src/Common/StorageProbeConstants.h",
    "src/Infrastructure/Logging/LogConstants.h",
    "src/Infrastructure/Conversion/ConversionConstants.h",
    "src/Infrastructure/Hardware/HardwareConstants.h", "src/Common/HardwareDiscoveryConstants.h",
}
PLACEHOLDERS = re.compile(r"\{[^{}]+\}")


def main():
    failures = []
    count = 0
    for path in sorted((ROOT / "src").rglob("*")):
        if path.suffix not in {".cpp", ".h"}:
            continue
        count += 1
        source = path.read_text(encoding="utf-8")
        definitions = path.relative_to(ROOT).as_posix() in CONSTANT_HEADERS
        for match in TOKEN.finditer(source):
            token = match.group()
            prefix = source[source.rfind("\n", 0, match.start()) + 1:match.start()]
            if token.startswith(("//", "/*")) or prefix.lstrip().startswith("#"):
                continue
            if definitions:
                if not prefix.strip().startswith("inline constexpr"):
                    failures.append(f"{path.relative_to(ROOT)}: nonconstant definition: {token}")
                continue
            if token == '"C"' and prefix.strip().endswith("extern"):
                continue  # Linkage syntax, not application data.
            if token in {"0", "1"}:
                continue  # Empty/unit values, indices, booleans, pure virtual declarations.
            failures.append(f"{path.relative_to(ROOT)}:{source.count(chr(10), 0, match.start()) + 1}: raw literal {token}")
    catalogs = ROOT / "src/Translations/Catalogs"
    base = json.loads((catalogs / "Constants/App/errors.json").read_text(encoding="utf-8"))["strings"]
    metadata = json.loads((ROOT / "src/Translations/Languages.json").read_text(encoding="utf-8"))
    invariants = [line.strip() for line in (ROOT / "scripts/translations/ignores/common.txt").read_text(encoding="utf-8").splitlines()
                  if line.strip() and not line.lstrip().startswith("#")]
    for language in metadata:
        path = catalogs / "Languages" / language["code"] / "App/errors.json"
        values = json.loads(path.read_text(encoding="utf-8"))["strings"]
        if values.keys() != base.keys():
            failures.append(f"{language['code']}: inconsistent error keys")
            continue
        for key, text in values.items():
            if not text.strip() or text == "Name" or sorted(PLACEHOLDERS.findall(text)) != sorted(PLACEHOLDERS.findall(base[key])):
                failures.append(f"{language['code']}: invalid error translation {key}")
            for invariant in invariants:
                if base[key].count(invariant) != text.count(invariant):
                    failures.append(f"{language['code']}: changed invariant {invariant} in {key}")
    keys = (ROOT / "src/Common/ErrorKeys.h").read_text(encoding="utf-8")
    for key in re.findall(r'"(Errors\.[^"\\]+)', keys):
        if key not in base:
            failures.append(f"Untranslated error key: {key}")
    english = json.loads((catalogs / "Languages/en-US/App/errors.json").read_text(encoding="utf-8"))["strings"]
    if english != base:
        failures.append("en-US errors differ from Constants")
    if failures:
        raise SystemExit("\n".join(failures))
    print(f"PASS: {count} C++ files, {len(base)} error keys, {len(metadata)} languages; no application literals outside named constants.")


if __name__ == "__main__":
    main()
