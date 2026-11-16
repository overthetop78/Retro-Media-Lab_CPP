# Modèle séquentiel, PCM entier et RAW audio

- [x] Modèle séquentiel
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/SequentialTypes.h`, `src/MediaEngine/src/Models/Sequential/Image.h`, `Image.cpp` : types, temps en ticks 100 ns, segments possédés, coordonnées dérivées et distinction collections absentes/vides.
- [x] PCM partagé
  - [x] Créer `src/MediaEngine/src/Formats/Tape/Common/PcmConstants.h`, `Pcm.h`, `PcmProfile.cpp`, `PcmSamples.cpp`, `PcmDocument.cpp` : profil RAW, PCM entier 8/16/24/32, extraction de canaux, pulsations/silence, normalisation, interpolation et codage sans perte à profil identique.
- [x] RAW audio
  - [x] Créer `src/MediaEngine/src/Formats/Tape/Raw/Reader.h`, `Reader.cpp`, `Writer.cpp` : choix explicite tape.raw, document séquentiel et publication atomique au profil demandé.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, `src/MediaEngine/src/Images/ReadRejection.cpp` et créer `src/Translations/Catalogs/Constants/MediaEngine/sequential-pcm.json`, `src/Translations/Catalogs/Languages/*/MediaEngine/sequential-pcm.json` : erreurs traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-sequential-pcm/CMakeLists.txt`, `main.cpp` : invariants du modèle, profils/échantillons, aller-retour RAW, canaux/précision/endianness/signe, interpolation, bornes et annulation ; sorties temporaires.
  - [x] Supprimer `build/verification-sequential-pcm/` avec ses projets, binaires et sorties temporaires après consignation.

## Validation

Contrôle ciblé, compilation Debug, audit et catalogues réussis ; références et limites dans docs/validation/media/sequential-pcm-raw.md. Temporaires supprimés.
