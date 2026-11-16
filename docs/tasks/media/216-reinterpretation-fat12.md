# Réinterprétation FAT12

- [x] Politique et écriture
  - [x] Créer `src/MediaEngine/src/Images/Conversion/Fat12.h` et `Fat12.cpp` : résolution géométrique Atari ST/IBM/MSX, refus hybride/incomplet/incompatible, validation BPB/layout et écriture sans migration ni modification des secteurs.
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/Fat12Conversion.h` et `src/MediaEngine/src/Images/Conversion/Fat12Facade.cpp` : contrat asynchrone sur document sectoriel et validation des pertes de structure.
  - [x] Modifier `Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp` et créer `Catalogs/Constants/MediaEngine/fat12-reinterpretation.json` et ses versions `Languages/*/MediaEngine/fat12-reinterpretation.json` : cinq refus traduits.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `.filters` : intégrer le bloc.
- [ ] Exploration et conversion par chemin
  - [ ] Modifier `src/MediaEngine/src/Images/Conversion/Fat12Facade.cpp` et `Fat12Conversion.h` après raccordement de l'explorateur de disques : conversion par chemin avec détection effective des interprétations hybrides du C#.
- [x] Validation
  - [x] Créer `build/verification-fat12-reinterpretation/CMakeLists.txt` et `main.cpp` : image FAT12 valide, destinations équivalentes, refus hybride/BPB/géométrie/secteurs manquants et annulation ; compiler le bloc et ses catalogues.
  - [x] Supprimer `build/verification-fat12-reinterpretation/` et ses médias après utilisation.
