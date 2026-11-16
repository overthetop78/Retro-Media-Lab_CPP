# Conteneur commun CHD

- [x] Structures et cartes
  - [x] Créer `src/MediaEngine/src/Formats/Common/Chd/ContainerConstants.h`, `Container.h`, `Container.cpp` : en-têtes v1-v5, dimensions, source possédée, métadonnées et lecture sérialisée des hunks.
  - [x] Créer `src/MediaEngine/src/Formats/Common/Chd/Map.cpp` : cartes historiques et v5, répétitions, références et CRC depuis le C#.
- [x] Composition des codecs
  - [x] Créer `src/MediaEngine/src/Formats/Common/Chd/ContainerCodecs.cpp`, `EccTables.h` : choix des codecs, entrelacement CD et régénération ECC depuis les tables C#.
  - [x] Modifier les diagnostics MediaEngine et créer `Catalogs/Constants/MediaEngine/chd-container.json` et les catalogues de langues correspondants : erreurs traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : référencer sources et constantes.
- [x] Validation ciblée
  - [x] Créer `build/verification-chd-container/CMakeLists.txt` et `main.cpp` : petits conteneurs synthétiques, cartes, métadonnées, codecs/CD, CRC, références, annulation.
  - [x] Supprimer `build/verification-chd-container/` et ses artefacts après consignation.
