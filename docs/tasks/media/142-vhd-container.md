# Conteneur VHD

- [x] Organisation et conteneur
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/ContainerConstants.h`, `Variant.h` et `Variant.cpp`, modifier `Vdi/Writer.cpp` : mutualiser la sélection fixe/dynamique conforme au C#.
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Vhd/Constants.h`, `Container.h`, `Header.cpp`, `Reader.cpp`, `Writing.h`, `Geometry.cpp`, `Writer.cpp` : footer, somme de contrôle, géométrie, BAT/bitmaps et lecture/écriture VHD autonome fixe/dynamique.
  - [x] Modifier les diagnostics MediaEngine et `src/MediaEngine/src/Images/ReadRejection.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/vhd.json` et `Languages/<langue>/MediaEngine/vhd.json` : erreurs traduites et rejets de reconnaissance ; modifier `scripts/translations/ignores/common.txt` pour les invariants.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : compiler et référencer les fichiers et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-vhd/CMakeLists.txt` et `main.cpp` : lecture/écriture fixe/dynamique, table/bitmaps, UUID, footer et checksums, bloc terminal, parent différentiel, validation des plages et annulation.
  - [x] Supprimer `build/verification-vhd/` avec ses données synthétiques et binaires après consignation.
