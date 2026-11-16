# Modèle séquentiel, PCM entier et RAW audio

- [ ] Modèle séquentiel
  - [ ] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/SequentialTypes.h`, `src/MediaEngine/src/Models/Sequential/Image.h`, `Image.cpp` : types, temps en ticks 100 ns, segments possédés, coordonnées dérivées et distinction collections absentes/vides.
- [ ] PCM partagé
  - [ ] Créer `src/MediaEngine/src/Formats/Tape/Common/PcmConstants.h`, `Pcm.h`, `PcmProfile.cpp`, `PcmSamples.cpp`, `PcmDocument.cpp` : profil RAW, PCM entier 8/16/24/32, extraction de canaux, pulsations/silence, normalisation, interpolation et codage sans perte à profil identique.
- [ ] RAW audio
  - [ ] Créer `src/MediaEngine/src/Formats/Tape/Raw/Reader.h`, `Reader.cpp`, `Writer.cpp` : choix explicite tape.raw, document séquentiel et publication atomique au profil demandé.
  - [ ] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, `src/MediaEngine/src/Images/ReadRejection.cpp` et créer `src/Translations/Catalogs/Constants/MediaEngine/sequential-pcm.json`, `src/Translations/Catalogs/Languages/*/MediaEngine/sequential-pcm.json` : erreurs traduisibles.
  - [ ] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : compilation et constantes.
- [ ] Contrôle ciblé
  - [ ] Créer `build/verification-sequential-pcm/CMakeLists.txt`, `main.cpp` : invariants du modèle, profils/échantillons, aller-retour RAW, canaux/précision/endianness/signe, interpolation, bornes et annulation ; sorties temporaires.
  - [ ] Supprimer `build/verification-sequential-pcm/` avec ses projets, binaires et sorties temporaires après consignation.

## Point de sauvegarde

Code du modèle séquentiel, fonctions PCM et lecteur/rédacteur RAW écrit et compilation Debug arrivée à la liaison de MediaEngine.dll. Validation ciblée, relecture/correction des 29 traductions et compilation des catalogues encore à terminer. Les cases de validation restent ouvertes. Les fichiers temporaires sous build/ ne sont pas versionnés.
