# Encodage ISO MFM, CRC et codes de taille

- [x] Primitives partagées
  - [x] Créer `src/MediaEngine/src/Codecs/Crc16.h`, `Crc16Constants.h`, `Crc16.cpp` : CRC 16 non réfléchi configurable, mise à jour et copie avec CRC.
  - [x] Créer `src/MediaEngine/src/Models/Sectors/SizeCodes.h`, `SizeCodes.cpp` et modifier `SectorConstants.h` : conversion taille/code ISO fidèle aux aides C# et diagnostics structurés.
- [x] Encodage MFM
  - [x] Créer `src/MediaEngine/src/Codecs/Iso/Mfm.h`, `MfmConstants.h`, `MfmEncoder.cpp`, `MfmFields.cpp`, `MfmInternal.h` : encodeur logique et physique complet, CHRN enregistrés, tailles enregistrées, données supprimées, CRC fourni/corrompu, capacité et annulation ; réutiliser le socle commun de pistes.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compilation, constantes et diagnostics traduisibles.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/iso-mfm-encoding.json` et les 29 catalogues de langue, modifier `scripts/translations/ignores/common.txt` : erreurs d’encodage/taille, génération Argos et relecture.
- [x] Validation
  - [x] Créer `build/verification-iso-mfm-encoding/Reference.csproj`, `Contracts.cs`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison au C# original des CRC/codes, cellules/intervalles logiques et physiques, métadonnées enregistrées, capacités tronquées/complètes, erreurs, annulation et traductions en Debug/Release.
  - [x] Supprimer `build/verification-iso-mfm-encoding/`, `build/iso-mfm-encoding-reference.log`, `build/iso-mfm-encoding-bench-debug.log`, `build/iso-mfm-encoding-bench-release.log` après validation.
