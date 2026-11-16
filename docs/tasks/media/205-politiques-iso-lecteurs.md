# Politiques ISO et lecteurs de reconstruction

- [x] ComplÃ©ter les politiques et les services de lecture
  - [x] CrÃ©er `src/MediaEngine/src/Reconstruction/Iso/Policy.h`, `Policy.cpp`, `ExplicitFamilies.cpp` : rÃ©solution ordonnÃ©e des familles, dÃ©codeurs associÃ©s et politiques explicites ADFS, Amstrad, gÃ©nÃ©rique, UCSD ; rÃ©utiliser les constructeurs sectoriels et constantes existants.
  - [x] Modifier `Reconstruction/Iso/Automatic.h`, `Automatic.cpp`, `RuntimeFamilies.cpp`, `RuntimeConstants.h` : partager la sonde IBM DOS, complÃ©ter IBM explicite/scan et BBC explicite sans modifier les rÃ¨gles automatiques.
  - [x] CrÃ©er `Reconstruction/Iso/Reader.h`, `Reader.cpp` et modifier `Preparation/ScpConversion.cpp` : lecture depuis chemin ou flux protÃ©gÃ©, cache existant, refus des collections vides et validation Atari ; utiliser ce service dans la prÃ©paration runtime.
  - [x] Modifier `Reconstruction/AmigaReader.h`, `AmigaReader.cpp` : progression typÃ©e des rÃ©volutions et pistes, total vÃ©rifiÃ©, arrÃªt sur erreur du rÃ©cepteur et annulation.
  - [x] Modifier les fichiers `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` et crÃ©er `Catalogs/Constants/MediaEngine/iso-policies.json`, `Catalogs/Languages/<langue>/MediaEngine/iso-policies.json` : diagnostics traduits des formats, gÃ©omÃ©tries, identitÃ© IBM et absence de candidats.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj`, `RetroMediaLab.vcxproj.filters` : intÃ©grer les sources et catalogues.
- [x] VÃ©rifier les nouvelles rÃ¨gles
  - [x] CrÃ©er `build/verification-iso-policies/CMakeLists.txt`, `main.cpp` et captures temporaires : routage/dÃ©codeurs, gÃ©omÃ©tries explicites, validation ADFS, ordre UCSD par piste, IBM scan et progression/caches ; compiler MediaEngine et les catalogues.
  - [x] Supprimer `build/verification-iso-policies/` et les journaux temporaires associÃ©s aprÃ¨s vÃ©rification.
