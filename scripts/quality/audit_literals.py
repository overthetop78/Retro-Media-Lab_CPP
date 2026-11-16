"""Audit application literals and error catalogs; no external files or media required."""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TOKEN = re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|(?:L|u8)?"(?:\\.|[^"\\])*"|(?:L)?\x27(?:\\.|[^\x27\\])*\x27|(?<![\w.])(?:0x[\da-fA-F]+|\d+\.\d+|\.\d+|\d+)(?![\w.])')
CONSTANT_HEADERS = {
    "src/MediaEngine/src/Formats/HardDisk/Adaptec/Constants.h",
    "src/MediaEngine/src/Formats/HardDisk/Acorn/Constants.h",
    "src/MediaEngine/src/Formats/HardDisk/ContainerConstants.h",
    "src/MediaEngine/src/Formats/HardDisk/Vhd/Constants.h",
    "src/MediaEngine/src/Formats/HardDisk/Vdi/Constants.h",
    "src/MediaEngine/src/Formats/KryoFlux/Constants.h",
    "src/MediaEngine/src/Formats/HxcStream/Constants.h",
    "src/MediaEngine/src/Formats/I86f/Constants.h",
    "src/MediaEngine/src/Formats/Cp2/Constants.h",
    "src/MediaEngine/src/Formats/Jfd/Constants.h",
    "src/MediaEngine/src/Formats/Catalog/Constants.h",
    "src/MediaEngine/src/Formats/Definitions/Constants.h",
    "src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FatBootSectorLayout.h",
    "src/MediaEngine/src/Formats/Atari/Constants.h",
    "src/MediaEngine/src/Formats/Optical/Iso/Constants.h",
    "src/MediaEngine/src/Models/Optical/Constants.h",
    "src/MediaEngine/src/Formats/Cartridge/Constants.h",
    "src/MediaEngine/src/Images/Constants.h",
    "src/MediaEngine/src/Models/Blocks/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Optical/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Iso9660/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Udf/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Xbox/Xdvdfs/Constants.h",

    "src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/MetadataNumberConstants.h",
    "src/MediaFileSystems/src/FileSystems/Nintendo/GameCube/Constants.h",
    "src/MediaFileSystems/src/Primitives/MetadataConstants.h",
    "src/MediaFileSystems/src/FileSystems/Nintendo/FamicomDisk/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Cartridge/Constants.h",
    "src/MediaEngine/src/Encoding/PackingConstants.h",
    "src/MediaEngine/src/Formats/Apple/TrackEncodingConstants.h",
    "src/MediaFileSystems/src/FileSystems/Apple/Macintosh/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Apple/Macintosh/MacRomanTable.h",
    "src/MediaEngine/src/Formats/Hfe/Constants.h",
    "src/MediaEngine/src/Formats/Apridisk/Constants.h",
    "src/MediaEngine/src/Formats/TeleDisk/Constants.h",
    "src/MediaEngine/src/Formats/Rx02/Constants.h",
    "src/MediaEngine/src/Formats/Xfd/Constants.h",
    "src/MediaEngine/src/Formats/AcornAtom/Constants.h",
    "src/MediaEngine/src/Formats/HardDisk/Raw/Constants.h",
    "src/MediaFileSystems/src/Exploration/Composition/Constants.h",
    "src/MediaFileSystems/src/Exploration/Composition/CatalogConstants.h",
    "src/MediaFileSystems/src/FileSystems/Atari/KFile/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Atari/BootDisk/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Atari/ClkGraphicsLibrary/Constants.h",
    "src/MediaEngine/src/Formats/Epson/Constants.h",
    "src/MediaEngine/src/Formats/ImageDisk/Constants.h",
    "src/MediaEngine/src/Formats/CpcDsk/Constants.h",
    "src/MediaEngine/src/Formats/Atari8Bit/Constants.h",
    "src/MediaEngine/src/Formats/Atr/Constants.h",
    "src/MediaEngine/src/Formats/Atx/Constants.h",
    "src/MediaEngine/src/Formats/Msa/Constants.h",
    "src/MediaEngine/src/Formats/St/Constants.h",
    "src/MediaEngine/src/Models/Flux/ConversionConstants.h",
    "src/MediaFileSystems/src/FileSystems/Apple/Lisa/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Ucsd/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Cpm/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Atari/Dos/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Atari/SpartaDos/Constants.h",
    "src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/AtariDosConstants.h",
    "src/MediaFileSystems/src/FileSystems/Dec/Rt11/Constants.h",
    "src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/CpmLayoutConstants.h",
    "src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FileSystemIds.h",
    "src/MediaFileSystems/src/FileSystems/Apple/Macintosh/Hfs/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Apple/Macintosh/Mfs/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Apple/ProDos/Constants.h",
    "src/MediaEngine/src/Formats/Woz/Constants.h",
    "src/MediaEngine/src/Formats/Apple/TrackSelectionConstants.h",
    "src/MediaEngine/src/Formats/TwoImg/Constants.h",
    "src/MediaEngine/src/Formats/Nib/Constants.h",
    "src/MediaEngine/src/Formats/DiskCopy/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Apple/Dos/Constants.h",
    "src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/AppleRecognitionConstants.h",
    "src/MediaEngine/src/Formats/Apple/Constants.h",
    "src/MediaEngine/src/Formats/Apple/GeometryConstants.h",
    "src/MediaEngine/src/Formats/Scp/DiskTypeConstants.h",
    "src/MediaEngine/src/Reconstruction/Sector/Constants.h",
    "src/MediaEngine/src/Primitives/IdentifierConstants.h",
    "src/MediaEngine/src/Formats/FormatIds.h",
    "src/MediaEngine/src/Encoding/TimingConstants.h",
    "src/MediaEngine/src/Codecs/Tycom/Constants.h",
    "src/MediaEngine/src/Codecs/QdMo5/Constants.h",
    "src/MediaEngine/src/Codecs/Membrain/Constants.h",
    "src/MediaEngine/src/Codecs/Arburg/Constants.h",
    "src/MediaEngine/src/Codecs/Aed6200p/Constants.h",
    "src/MediaEngine/src/Codecs/Centurion/Constants.h",
    "src/MediaEngine/src/Codecs/HpMmfm/Constants.h",
    "src/MediaEngine/src/Codecs/FmAddressMarkConstants.h",
    "src/MediaEngine/src/Codecs/DecRx02/Constants.h",
    "src/MediaEngine/src/Codecs/DataGeneral/Constants.h",
    "src/MediaEngine/src/Codecs/Heathkit/Constants.h",
    "src/MediaEngine/src/Primitives/BitConstants.h",
    "src/MediaEngine/src/Codecs/Northstar/Constants.h",
    "src/MediaEngine/src/Codecs/RotatingChecksumConstants.h",
    "src/MediaEngine/src/Codecs/Apple/Constants.h",
    "src/MediaEngine/src/Codecs/AppleII/Constants.h",
    "src/MediaEngine/src/Codecs/AppleRwts18/Constants.h",
    "src/MediaEngine/src/Codecs/AppleIwm/Constants.h",
    "src/MediaEngine/src/Codecs/Victor9000/Constants.h",
    "src/MediaEngine/src/Codecs/MicralN/Constants.h",
    "src/MediaEngine/src/Codecs/Emu/Constants.h",
    "src/MediaEngine/src/Codecs/Micropolis/Constants.h",
    "src/MediaEngine/src/Codecs/Amiga/Constants.h",
    "src/MediaEngine/src/Codecs/Raw/Constants.h",
    "src/MediaEngine/src/Decoding/ScoringConstants.h",
    "src/MediaEngine/src/Models/Flux/ProtectedConstants.h",
    "src/MediaEngine/src/Reconstruction/Iso/Constants.h",
    "src/MediaEngine/src/Codecs/Iso/FmConstants.h",
    "src/MediaEngine/src/Codecs/Crc16Constants.h",
    "src/MediaEngine/src/Codecs/Iso/MfmConstants.h",
    "src/MediaEngine/src/Codecs/Commodore/Commodore1541Constants.h",
    "src/MediaFileSystems/src/Primitives/PetsciiConstants.h",
    "src/MediaFileSystems/src/FileSystems/Commodore/Dos/Constants.h",
    "src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/CommodoreGeometryConstants.h",
    "src/MediaEngine/src/Formats/CommodoreDisks/Constants.h",
    "src/MediaEngine/src/Encoding/Constants.h",
    "src/MediaEngine/src/Formats/Scp/Constants.h",
    "src/MediaEngine/src/Codecs/Commodore/Commodore900Constants.h",
    "src/MediaEngine/src/Models/Flux/Constants.h",
    "src/MediaEngine/src/Decoding/Constants.h",
    "src/MediaEngine/src/Codecs/Commodore/Constants.h",
    "src/MediaEngine/src/Formats/Commodore900/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Commodore/Coherent/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Amiga/FlatArchive/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Amiga/Dos/Constants.h",
    "src/MediaEngine/src/Formats/Adf/Constants.h",
    "src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FileCoreLayout.h",
    "src/MediaFileSystems/src/FileSystems/Acorn/Adfs/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Acorn/FileCore/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Acorn/BbcDfs/Constants.h",
    "src/MediaEngine/src/Formats/BbcDfs/Constants.h",
    "src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FormatIds.h",
    "src/MediaEngine/include/RetroMediaLab/MediaEngine/DiagnosticKeys.h",
    "src/MediaEngine/src/Formats/Raw/Constants.h",
    "src/MediaEngine/src/Sources/AtomicConstants.h",
    "src/MediaFileSystems/src/Primitives/BinaryConstants.h",
    "src/MediaFileSystems/src/Primitives/MetadataConstants.h",
    "src/MediaFileSystems/src/FileSystems/Fat/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Fat/Fat12/Constants.h",
    "src/MediaFileSystems/src/FileSystems/Fat/Fat12/FormatCatalog.h",
    "src/MediaFileSystems/src/Exploration/AnalysisConstants.h",
    "src/MediaEngine/src/Models/Sectors/SectorConstants.h",

    "src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/DiagnosticKeys.h",
    "src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/VolumeConstants.h",
    "src/MediaFileSystems/src/Partitioning/PartitionConstants.h",
    "src/MediaEngine/src/Sources/SourceConstants.h",

    "src/MediaAnalysis/include/RetroMediaLab/MediaAnalysis/Localization.h",
    "src/MediaAnalysis/src/Catalogs/Amiga.h",
    "src/MediaAnalysis/src/Catalogs/AppleDos.h",
    "src/MediaAnalysis/src/Catalogs/Atari8Bit.h",
    "src/MediaAnalysis/src/Catalogs/AtariCommon.h",
    "src/MediaAnalysis/src/Catalogs/AtariTos.h",
    "src/MediaAnalysis/src/Catalogs/BbcMicro.h",
    "src/MediaAnalysis/src/Catalogs/Commodore.h",
    "src/MediaAnalysis/src/Catalogs/Common.h",
    "src/MediaAnalysis/src/Catalogs/Cpm.h",
    "src/MediaAnalysis/src/Catalogs/Dec.h",
    "src/MediaAnalysis/src/Catalogs/IbmPc.h",
    "src/MediaAnalysis/src/Catalogs/Lisa.h",
    "src/MediaAnalysis/src/Catalogs/Macintosh.h",
    "src/MediaAnalysis/src/Catalogs/Msx.h",
    "src/MediaAnalysis/src/Catalogs/ProDos.h",
    "src/MediaAnalysis/src/Catalogs/Ucsd.h",
    "src/MediaAnalysis/src/Constants/Extensions.h",
    "src/MediaAnalysis/src/Constants/Presentation.h",
    "src/MediaAnalysis/src/Constants/RecognitionConstants.h",
    "src/MediaAnalysis/src/Constants/Signatures.h",

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
        constant_definitions = []
        if definitions:
            # Ignore semicolons inside strings/comments when finding the end of
            # a named constant initializer, including multiline array tables.
            syntax = TOKEN.sub(lambda item: " " * len(item.group()) if item.group().startswith(
                ('//', '/*', '"', "'", 'L"', 'u8"', "L'")) else item.group(), source)
            for declaration in re.finditer(
                    r"\binline\s+constexpr\s+[^;{}()=]*\b[A-Za-z_]\w*(?:\s*\[[^\[\]]*\])*\s*(?=[={])", syntax):
                end = syntax.find(";", declaration.end())
                if end >= 0:
                    constant_definitions.append((declaration.start(), end))
        enum_definitions = [match.span() for match in re.finditer(
            r"\benum(?:\s+(?:class|struct))?\s+\w+(?:\s*:\s*[\w:]+)?\s*\{[^{}]*\}", source)]
        for match in TOKEN.finditer(source):
            token = match.group()
            prefix = source[source.rfind("\n", 0, match.start()) + 1:match.start()]
            if token.startswith(("//", "/*")) or prefix.lstrip().startswith("#"):
                continue
            if definitions:
                if not any(start <= match.start() < end for start, end in constant_definitions):
                    failures.append(f"{path.relative_to(ROOT)}: nonconstant definition: {token}")
                continue
            if token == '"C"' and prefix.strip().endswith("extern"):
                continue  # Linkage syntax, not application data.
            if re.fullmatch(r"(?:0x[\da-fA-F]+|\d+)", token) and any(
                    start <= match.start() < end for start, end in enum_definitions):
                continue  # An enum member is itself a named integral constant.
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
    for component in ("MediaEngine", "MediaFileSystems"):
        diagnostic_keys = (ROOT / f"src/{component}/include/RetroMediaLab/{component}/DiagnosticKeys.h").read_text(encoding="utf-8")
        translated = {}
        for source in (catalogs / "Constants" / component).glob("*.json"):
            catalog = json.loads(source.read_text(encoding="utf-8"))
            entries = catalog["strings"]
            isolated = set(catalog.get("invariants", []))
            if not isolated <= entries.keys():
                failures.append(f"{component}/{source.name}: unknown invariant keys")
            expected = {key: value for key, value in entries.items() if key not in isolated}
            if translated.keys() & entries.keys():
                failures.append(f"{component}: duplicate diagnostics across catalogs")
            translated.update(entries)
            for language in metadata:
                target = catalogs / "Languages" / language["code"] / component / source.name
                if not target.exists():
                    failures.append(f"Missing {target.relative_to(ROOT)}")
                    continue
                values = json.loads(target.read_text(encoding="utf-8"))["strings"]
                if values.keys() != expected.keys():
                    failures.append(f"{target.relative_to(ROOT)}: inconsistent diagnostic keys")
                for key, original in expected.items():
                    value = values.get(key, "")
                    if not value.strip() or sorted(PLACEHOLDERS.findall(value)) != sorted(PLACEHOLDERS.findall(original)) or value.count("{") != original.count("{") or value.count("}") != original.count("}") or re.search(r"ZXQ[0-9]+QX", value):
                        failures.append(f"{target.relative_to(ROOT)}: invalid diagnostic {key}")
                    for invariant in invariants:
                        if original.count(invariant) != value.count(invariant):
                            failures.append(f"{target.relative_to(ROOT)}: changed invariant {invariant} in {key}")
                if language["code"] == "en-US" and values != expected:
                    failures.append(f"{component}/{source.name}: en-US differs from Constants")
        for key in re.findall(r'"((?:Media|MediaEngine|MediaFileSystems)\.[^"\\]+)', diagnostic_keys):
            if key not in translated:
                failures.append(f"{component}: untranslated diagnostic {key}")
    if failures:
        raise SystemExit("\n".join(failures))
    print(f"PASS: {count} C++ files, {len(base)} error keys, {len(metadata)} languages; no application literals outside named constants.")


if __name__ == "__main__":
    main()
