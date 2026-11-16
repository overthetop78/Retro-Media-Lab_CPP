# Acquisition et préparation physiques

- [x] Contrats neutres et services
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/PhysicalMediaTypes.h`, `PhysicalMediaIds.h` et `PhysicalMedia.h` : contrats possédés, interfaces fournisseurs, métadonnées et préparation sans dépendance matérielle publique.
  - [x] Créer `src/MediaEngine/src/PhysicalMedia/Contracts.cpp`, `Constants.h`, `Tape.cpp`, `Floppy.cpp` et `FloppyPlanning.cpp` : validations C#, PCM16 sans transformation, reconstruction SCP et plans ordonnés d'écriture.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` et les catalogues `src/Translations/Catalogs/{Constants,Languages/*}/MediaEngine/physical.json` : erreurs traduisibles de contrats physiques.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : intégrer le bloc.
- [x] Vérification du bloc
  - [x] Créer `build/verification-physical-media/CMakeLists.txt` et `main.cpp` : contrats, PCM fragmenté, flux et plans sans accès matériel.
  - [x] Supprimer `build/verification-physical-media/` et ses artefacts après utilisation.
