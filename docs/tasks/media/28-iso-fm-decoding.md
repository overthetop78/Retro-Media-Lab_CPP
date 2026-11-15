# Décodage ISO FM

- [x] Champs et structures
  - [x] Modifier `src/MediaEngine/src/Codecs/Iso/Fm.h`, `FmConstants.h` ; créer `FmRead.h`, `FmRead.cpp`, `FmDecoder.cpp` : reconnaissance des marques, horloges et CRC, métadonnées enregistrées, appairage et ordre des structures/octets, périodes de recherche source.
- [x] Reconstruction temporelle
  - [x] Créer `src/MediaEngine/src/Codecs/Iso/FmTiming.cpp` : décodage adaptatif et décodage à durée connue, arrondi pair, validation index/intervalles, répétition bornée de la période et annulation.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/iso-fm-decoding.json` et les 29 catalogues : descriptions et erreur temporelle, génération Argos et relecture/réemploi des traductions MFM identiques hormis l’invariant.
- [x] Validation
  - [x] Créer `build/verification-iso-fm-decoding/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison aux sources C# originales des marques/champs/horloges, décodages adaptatif/fixe/cellules, circularité, CRC enregistrés, erreurs, annulation et textes en Debug/Release.
  - [x] Supprimer `build/verification-iso-fm-decoding/`, `build/iso-fm-decoding-reference.log`, `build/iso-fm-decoding-bench-debug.log`, `build/iso-fm-decoding-bench-release.log` : retrait des fichiers temporaires après validation.
