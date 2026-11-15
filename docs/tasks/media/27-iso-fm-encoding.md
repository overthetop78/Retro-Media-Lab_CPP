# Encodage ISO FM

- [x] Codec d’écriture
  - [x] Créer `src/MediaEngine/src/Codecs/Iso/Fm.h`, `FmConstants.h`, `FmInternal.h`, `FmFields.cpp`, `FmEncoder.cpp` : marques FM spéciales, champs CHRN/CRC, métadonnées enregistrées, disposition logique/physique et temporalité source.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compilation du codec.
- [x] Diagnostics
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, `scripts/translations/ignores/common.txt` ; créer `src/Translations/Catalogs/Constants/MediaEngine/iso-fm-encoding.json` et les 29 catalogues : diagnostics de champs/taille, génération Argos et relecture.
- [x] Audit des constantes
  - [x] Modifier `scripts/quality/audit_literals.py` : déclarer `src/MediaEngine/src/Codecs/Iso/FmConstants.h` comme en-tête de constantes nommé, comme les autres codecs.
- [x] Validation
  - [x] Créer `build/verification-iso-fm-encoding/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison à l’encodeur C# original des bits/intervalles, champs et CRC, tailles, attributs physiques, troncatures, refus et annulation, textes en Debug/Release.
  - [x] Supprimer `build/verification-iso-fm-encoding/`, `build/iso-fm-encoding-reference.log`, `build/iso-fm-encoding-bench-debug.log`, `build/iso-fm-encoding-bench-release.log` : retrait des fichiers temporaires après vérification.
