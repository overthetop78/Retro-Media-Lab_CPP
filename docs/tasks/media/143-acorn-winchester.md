# Images Acorn Winchester DAT/DSC

- [x] Descripteur et paramètres physiques
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Adaptec/Constants.h`, `Parameters.h`, `Parameters.cpp` : projection des paramètres initiaux Adaptec nécessaires au DSC, sans anticiper le codec de pistes restant.
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Acorn/Constants.h`, `Descriptor.h`, `Descriptor.cpp` : géométrie DSC, compatibilité, conservation et mise à jour des descripteurs, projection des paramètres physiques.
- [x] Lecture et publication
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Acorn/Container.h`, `Reader.cpp`, `Writer.cpp` : reconnaissance DAT/DSC et Hugo, plages partielles indisponibles, métadonnées natives, publication atomique de la paire, création sans remplacement et préallocation optionnelle.
  - [x] Modifier les diagnostics MediaEngine et `src/MediaEngine/src/Images/ReadRejection.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/acorn-winchester.json` et `Languages/<langue>/MediaEngine/acorn-winchester.json` : erreurs et messages traduits ; modifier les invariants communs si nécessaire.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : compiler et référencer les nouveaux fichiers et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-acorn-winchester/CMakeLists.txt` et `main.cpp` : géométries séquentielles/interleavées, conservation DSC, paramètres Adaptec, lecture partielle, signature Hugo et création/publication de paire.
  - [x] Supprimer `build/verification-acorn-winchester/` avec ses données synthétiques et binaires après consignation.
