# Codec E-mu FM

- [x] Pistes et champs E-mu
  - [x] Créer `src/MediaEngine/src/Codecs/Emu/Constants.h`, `Fm.h`, `Encoder.cpp`, `Decoder.cpp` : adresse inversée, FM 32 cellules, CRC IBM commun, données et marques classées/non classées, structures triées selon le C#.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compilation, erreurs et constantes nommées.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/emu-fm.json` et les 29 catalogues : erreurs et descriptions localisées, génération Argos puis relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger E-mu et E-mu Emulator dans toutes les langues.
- [x] Validation
  - [x] Créer `build/verification-emu/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : sources originales, coordonnées inversées, CRC, secteurs et numéros ignorés, horloges, corruption/troncature, recherche et classification des marques ; langues et annulation Debug/Release.
  - [x] Supprimer `build/verification-emu/` et les journaux de comparaison : retrait du banc et de ses artefacts après vérification.
