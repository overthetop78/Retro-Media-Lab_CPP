# Images brutes génériques définies par catalogue

- [x] Backend et raccordements
  - [x] Créer `src/MediaEngine/src/Formats/Definitions/RawImages.h` et `RawImages.cpp` : lire les secteurs et ignorer le rembourrage, construire l'image variable, vérifier la géométrie et écrire atomiquement les secteurs avec rembourrage nul ; fournir lecteur et adapter writer depuis un catalogue possédé.
  - [x] Modifier `src/MediaEngine/src/Formats/Definitions/Constants.h` et `src/MediaEngine/CMakeLists.txt` : nommer l'identifiant/extension et compiler le backend.
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`) et `Images/ReadRejection.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/defined-raw.json` et les équivalents `Languages/<langue>/MediaEngine/defined-raw.json` : erreurs traduites de format, longueur, géométrie et secteur ; classer seulement les erreurs de lecture prévues comme rejets.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-defined-raw/CMakeLists.txt` et `main.cpp` : vérifier lecture/écriture avec tailles différentes et rembourrage, sélection explicite du lecteur, refus des secteurs absents et préservation de la sortie en cas d'échec.
  - [x] Supprimer `build/verification-defined-raw/` et ses fichiers/binaires temporaires après consignation.
