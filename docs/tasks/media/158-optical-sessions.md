# Sessions optiques et audio associé

- [x] Contrats et audio associé
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/OpticalTrackMode.h`, modifier `src/MediaEngine/src/Models/Optical/Track.h` : déplacer l’énumération du mode optique dans le contrat public du moteur, sans exposer MFS.
  - [x] Créer `src/MediaEngine/src/Images/Optical/Constants.h`, `CompanionAudio.h`, `CompanionAudio.cpp` : vingt conventions OGG/WAV, profil CD, séquencement, padding, heuristique de pregap stocké, sources possédées et document résultant.
- [x] Cœur de session
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/OpticalDiscSession.h`, `src/MediaEngine/src/Images/Optical/DiscSession.cpp` : résumé des pistes sans MFS, boucles, secteurs/pregaps/sous-canaux, cache transactionnel, fermeture et nettoyage RAII.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` et créer `src/Translations/Catalogs/Constants/MediaEngine/optical-session.json`, `src/Translations/Catalogs/Languages/*/MediaEngine/optical-session.json` : erreurs traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-optical-session/CMakeLists.txt`, `main.cpp` : audio associé, séquences/pregaps/padding, résumé/boucles, lecture streaming/cache, annulation de précache, fermeture et durée de vie des sources.
  - [x] Supprimer `build/verification-optical-session/` avec ses projets, binaires et fixtures après consignation.
- [ ] Raccordement global (après les services de lecture/écriture)
  - [ ] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/OpticalDiscSession.h` et créer `src/MediaEngine/src/Images/Optical/OpenSession.cpp` : ouverture par chemin via la composition globale et audio associé facultatif, sans registre concurrent.
  - [ ] Créer `src/MediaEngine/src/Images/Conversion/Optical/ConversionService.h`, `ConversionService.cpp` : destinations du registre de writers, validation et conversion via les services globaux, progression.
