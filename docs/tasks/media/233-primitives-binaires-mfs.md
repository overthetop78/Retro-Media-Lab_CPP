# Primitives binaires communes MediaFileSystems

- [x] Regrouper les lectures binaires
  - [x] Modifier `src/MediaFileSystems/src/Primitives/Binary.h` et `BinaryConstants.h` : lecture little endian 24 bits bornée et opérations little endian signées par représentation non signée, en réutilisant les diagnostics existants.
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Apple/ProDos/Codec.cpp`, `src/MediaFileSystems/src/FileSystems/Acorn/Adfs/Directory.cpp`, `src/MediaFileSystems/src/FileSystems/Acorn/FileCore/Codec.cpp` et `OldMap.cpp` : remplacer les assemblages 24 bits répétés par la primitive commune, sans changer les interprétations des formats.
- [x] Vérification ciblée
  - [x] Créer `build/verification-mfs-binary/CMakeLists.txt` et `main.cpp` : lectures/écritures signées, valeurs 24 bits et bornes, sans retester les systèmes de fichiers.
  - [x] Supprimer `build/verification-mfs-binary/` et ses artefacts après utilisation.
