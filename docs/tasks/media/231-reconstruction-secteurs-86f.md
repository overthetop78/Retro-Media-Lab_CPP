# Reconstruction sectorielle 86F

- [x] Service de reconstruction
  - [x] Créer `src/MediaEngine/src/Formats/I86f/SectorReader.h` et `SectorReader.cpp` : conversion des cellules sans transition de fin ajoutée, sélection FM/MFM par drapeaux, collecte des candidats et construction ISO uniforme selon le C#.
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/I86fReconstruction.h` et `src/MediaEngine/src/Formats/I86f/SectorFacade.cpp` : ouverture asynchrone vers un document opaque, sans exposer MediaFileSystems à App.
  - [x] Modifier `src/MediaEngine/src/Formats/I86f/Constants.h`, `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : ajouter les identifiants techniques et intégrer le service.
- [x] Vérification du raccordement
  - [x] Créer `build/verification-i86f-sectors/CMakeLists.txt` et `main.cpp` : décodeurs injectés, sélection par drapeaux, temps des transitions, secteurs retenus, géométrie, absence de transition et annulation ; aucun retest des codecs.
  - [x] Supprimer `build/verification-i86f-sectors/` et ses artefacts après utilisation.
