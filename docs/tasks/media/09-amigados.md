# AmigaDOS — OFS/FFS, récupération et création de volumes

- [x] Contrats et primitives
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/AmigaDos.h` : variantes, codecs/noms/dates, lecture et création des volumes.
  - [x] Modifier `src/MediaFileSystems/src/Primitives/Binary.h` : lecture/écriture big-endian bornée réutilisable.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Amiga/Dos/Constants.h`, `Internal.h`, `Codec.cpp` : schéma, variantes, types, noms Latin-1, sommes de contrôle et dates.
- [x] Exploration et création
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Amiga/Dos/Reader.cpp`, `Directory.cpp`, `File.cpp`, `Recovery.cpp`, `Writer.cpp` : racine conventionnelle/déclarée, chaînes, OFS/FFS/extensions, bitmap, récupération et construction des secteurs.
  - [x] Modifier `src/MediaEngine/src/Formats/Adf/AdfImages.h` et `Reader.cpp` : exploration Amiga/Acorn sélectionnée selon le format, puis analyse des entrées.
- [x] Construction et traductions
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, `include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` et régénérer les projets Visual C++ : compilation et diagnostics AmigaDOS.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/amigados.json` et les équivalents des 29 langues : avertissements/erreurs traduits par Argos, puis relus.
- [x] Validation
  - [x] Créer `build/verification-amigados/` : banc compilant les sources C# originales et comparant variantes, noms/dates, volumes, récupération et octets des writers ; contrôles de corruption, annulation et limites.
  - [x] Supprimer `build/verification-amigados/` et ses artefacts après validation Debug et Release.
