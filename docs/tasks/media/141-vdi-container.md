# Conteneur de disque dur VDI

- [x] Sources et conteneur
  - [x] Modifier `src/MediaEngine/src/Sources/AtomicImageWriter.h` et `.cpp` : ajouter une sortie atomique positionnable réutilisable, sans modifier les producteurs séquentiels existants.
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Vdi/Constants.h`, `Container.h`, `Header.cpp`, `Reader.cpp`, `Writer.cpp` : lecture VDI 1.1 autonome fixe/dynamique, table des blocs, UUID, géométrie et écriture atomique conforme au C#.
  - [x] Modifier les diagnostics MediaEngine, créer `src/Translations/Catalogs/Constants/MediaEngine/vdi.json` et `Languages/<langue>/MediaEngine/vdi.json` : erreurs traduites du conteneur et du positionnement atomique ; modifier `scripts/translations/ignores/common.txt` pour protéger VDI.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : compiler et référencer les nouveaux fichiers et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-vdi/CMakeLists.txt` et `main.cpp` : lecture/écriture fixe et dynamique, blocs stockés/nuls/non alloués, UUID, géométrie, bloc terminal, validations des en-têtes/tables et annulation atomique.
  - [x] Supprimer `build/verification-vdi/` et ses données synthétiques/binaires après consignation.
