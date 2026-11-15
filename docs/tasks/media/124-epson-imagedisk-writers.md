# Writers Epson brut et ImageDisk

- [x] Backend Epson brut
  - [x] Créer `src/MediaEngine/src/Formats/Epson/RawWriter.h` et `RawWriter.cpp` : écrire les secteurs dans l’ordre du catalogue variable Epson, conserver les pistes vides, contrôler géométrie, doublons, présence et taille ; utiliser l’écriture atomique commune.
- [x] Adapters
  - [x] Créer `src/MediaEngine/src/Formats/Epson/WriterAdapter.h` et `WriterAdapter.cpp`, modifier `Epson/Constants.h` : choisir IMG brut ou IMD restauré/construit selon l’extension et le profil cible.
  - [x] Créer `src/MediaEngine/src/Formats/ImageDisk/WriterAdapter.h` et `WriterAdapter.cpp`, modifier `ImageDisk/Constants.h` : réécrire un IMD depuis les métadonnées détaillées du document, sans reconstruire silencieusement une structure absente.
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`) et créer les catalogues `Constants/MediaEngine/epson-raw-writing.json` et `Languages/<langue>/MediaEngine/epson-raw-writing.json` sous `src/Translations/Catalogs/` : erreurs Epson traduites avec leurs coordonnées et longueurs.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler backend et adapters.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-epson-imagedisk-writers/CMakeLists.txt` et `main.cpp` : vérifier une sortie brute Epson variable, le refus des doublons/secteurs absents et la conservation de la structure/commentaire IMD par les adapters.
  - [x] Supprimer `build/verification-epson-imagedisk-writers/` et ses sorties synthétiques après consignation.
