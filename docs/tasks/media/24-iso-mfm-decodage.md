# Décodage ISO MFM et sélection PLL

- [x] Lecture des cellules
  - [x] Créer `src/MediaEngine/src/Models/Flux/BitReader.h`, `BitReader.cpp` : motifs bornés, octets bruts/MFM/FM et séquences MFM ; aucun accès hors plage, annulation et limite de nombre d’octets.
- [x] Décodage MFM
  - [x] Modifier `src/MediaEngine/src/Codecs/Iso/Mfm.h`, `MfmConstants.h` et créer `MfmRead.h`, `MfmRead.cpp`, `MfmDecoder.cpp`, `MfmPll.cpp` : collecte de marques, champs/CRC, queue circulaire, secteurs et métadonnées d’intégrité, ordre des structures/octets, descriptions et sélection stable des sept tentatives PLL.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` : compilation et diagnostic de nombre d’octets invalide.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/iso-mfm-decoding.json` et les 29 catalogues de langue, modifier `scripts/translations/ignores/common.txt` : descriptions et erreur traduisibles, génération Argos et relecture.
- [x] Validation
  - [x] Créer `build/verification-iso-mfm-decoding/Reference.csproj`, `Contracts.cs`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison aux sources C# originales pour lecteur binaire, champs, décodage complet/circulaire/PLL, erreurs et métadonnées, scores et ordre de sélection, annulation et traductions en Debug/Release.
  - [x] Supprimer `build/verification-iso-mfm-decoding/`, `build/iso-mfm-decoding-reference.log`, `build/iso-mfm-decoding-bench-debug.log`, `build/iso-mfm-decoding-bench-release.log` après validation.
