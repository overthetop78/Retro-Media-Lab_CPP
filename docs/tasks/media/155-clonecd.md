# CloneCD CCD/IMG/SUB

- [x] Descripteur possédé
  - [x] Créer `src/MediaEngine/src/Formats/Optical/CloneCd/Constants.h` : noms INI, versions 2/3, tailles, modes et métadonnées du C#.
  - [x] Créer `src/MediaEngine/src/Formats/Optical/CloneCd/Descriptor.h`, `Descriptor.cpp` : TOC, sessions et pistes immuables ; lecture Unicode, sections/propriétés insensibles à la casse, entiers décimaux/hexadécimaux et validations.
- [x] Lecture optique
  - [x] Créer `src/MediaEngine/src/Formats/Optical/CloneCd/Reader.h`, `Reader.cpp` : reconnaissance CCD, IMG obligatoire, SUB optionnel, sessions, fenêtres utilisateur, index et pregaps stockés ; sources partagées sans handle persistant.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, `src/MediaEngine/src/Images/ReadRejection.cpp` et créer `src/Translations/Catalogs/Constants/MediaEngine/clonecd.json`, `src/Translations/Catalogs/Languages/*/MediaEngine/clonecd.json` : erreurs traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : sources, constantes et navigation Visual C++.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger CloneCD et SUB pour toutes les langues.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-clonecd/CMakeLists.txt`, `main.cpp` : sessions/TOC/entiers, IMG/SUB, pistes et index/pregaps, reconnaissance négative et annulation ; fixtures synthétiques temporaires.
  - [x] Supprimer `build/verification-clonecd/` et ses binaires, CCD/IMG/SUB temporaires après consignation.
