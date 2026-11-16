# Protocole Atom séquentiel

- [x] Adaptateurs de décodage et encodage
  - [x] Créer `src/MediaEngine/src/Decoding/Sequential/Acorn/AtomDecoder.h`, `AtomDecoder.cpp` : sélection Atom TAP/UEF/PCM, décodage série ou FSK, rattachement des positions source et conservation des segments non résolus.
  - [x] Créer `src/MediaEngine/src/Encoding/Sequential/Acorn/AtomEncoder.h`, `AtomEncoder.cpp` : reconstruction TAP/UEF et PCM au débit choisi, borne mémoire et métadonnées de reconstruction.
  - [x] Modifier `src/MediaEngine/src/Formats/Tape/AtomTap/Constants.h`, `Codec.h`, `Encode.cpp` : identifiants du protocole et validation des fichiers sans recopier leurs données pour la sélection d’un encodeur.
  - [x] Modifier `src/MediaEngine/src/Decoding/Sequential/Models.h`, `Models.cpp` : rattacher de nouvelles positions source en conservant le partage des octets immuables.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/atom-protocol.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/atom-protocol.json` : erreurs et messages traduits.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj`, `.filters` : compilation des adaptateurs.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-atom-protocol/CMakeLists.txt`, `main.cpp` : sélection via registres, aller-retour TAP/UEF/PCM, FSK, conservation des erreurs, repères partagés, profil/limite et annulation.
  - [x] Supprimer `build/verification-atom-protocol/` et ses artefacts temporaires après consignation.
