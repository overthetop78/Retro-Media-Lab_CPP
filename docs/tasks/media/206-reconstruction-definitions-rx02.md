# Reconstruction SCP par dÃ©finition et DEC RX02

- [x] Lecteurs et gÃ©omÃ©trie partagÃ©e
  - [x] Modifier `src/MediaEngine/src/Formats/Definitions/RawImages.h`, `RawImages.cpp`, `Constants.h` : extraire la construction de l'image dÃ©finie (tailles, positions manquantes, capacitÃ© et blocs logiques) pour la partager entre IMG et SCP ; nommer les paramÃ¨tres de face et le nombre de secteurs NorthStar.
  - [x] CrÃ©er `src/MediaEngine/src/Reconstruction/DefinedReader.h`, `DefinedReader.cpp` : catalogue existant, routage codec, Micropolis complet, filtre face/taille, NorthStar matÃ©riel et progression du collecteur existant ; lectures par chemin et flux protÃ©gÃ©.
  - [x] CrÃ©er `src/MediaEngine/src/Reconstruction/Rx02Reader.h`, `Rx02Reader.cpp` : correspondance physique/logique existante, meilleurs candidats, paires de 256 octets, intÃ©gritÃ© combinÃ©e, rÃ©volution maximale, positions absentes et progression par piste.
  - [x] Modifier `Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp` et crÃ©er `Catalogs/Constants/MediaEngine/defined-scp.json`, `Catalogs/Languages/<langue>/MediaEngine/defined-scp.json` : erreur de codec natif indisponible traduite.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj`, `RetroMediaLab.vcxproj.filters` : compiler les lecteurs et intÃ©grer les catalogues.
- [x] VÃ©rification ciblÃ©e
  - [x] CrÃ©er `build/verification-defined-scp/CMakeLists.txt`, `main.cpp` et donnÃ©es temporaires : dÃ©finitions mixtes, face physique/enregistrÃ©e, Micropolis 256/275, parcours NorthStar, paires RX02/intÃ©gritÃ©/rÃ©volutions, progression, erreurs et annulation ; compiler les sources et catalogues.
  - [x] Supprimer `build/verification-defined-scp/` et les journaux de vÃ©rification temporaires aprÃ¨s utilisation.
