# Création et intégration JFD

- [x] Création et compression
  - [x] Modifier `src/MediaEngine/src/Formats/Jfd/Constants.h` et `Jfd.h`, créer `Create.cpp` : création de tables natives à partir des secteurs représentables, politiques de codage FM/MFM et densité, conservation des ordres et intégrités.
  - [x] Modifier `src/MediaEngine/src/Formats/Jfd/Compression.cpp` : compression gzip avec zlib embarquée, annulation et nettoyage RAII.
- [x] Registres communs
  - [x] Créer `src/MediaEngine/src/Formats/Jfd/Adapter.h`, `Reader.cpp`, `Writer.cpp` et `Metadata.cpp` : lecteur injectable, reconnaissance, deux messages informatifs traduits, métadonnées natives strictes en base64, writer atomique utilisant les tables conservées ou la création neuve ; mode gzip conservé.
  - [x] Modifier `src/MediaEngine/src/Images/ReadRejection.cpp`, les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`) et créer `src/Translations/Catalogs/Constants/MediaEngine/jfd-adapters.json` et ses équivalents `Languages/<langue>/MediaEngine/` : erreurs et informations traduites, rejet contrôlé des structures invalides dans le registre.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` et les projets/filtres Visual C++ générés : compiler les nouveaux fichiers.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger les noms techniques DiscOp, base64 et gzip ; corriger la casse de gzip dans les catalogues `Languages/pl-PL/MediaEngine/jfd-native.json` et `Languages/ro-RO/MediaEngine/jfd-native.json`.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-jfd-adapters/CMakeLists.txt` et `main.cpp` : création neuve, reconnaissance/lecture/écriture du registre, raw/gzip, secteurs variables et ordre, métadonnées invalides et fichier existant préservé lors d'une erreur.
  - [x] Supprimer `build/verification-jfd-adapters/` et ses fichiers/binaires temporaires après consignation.
