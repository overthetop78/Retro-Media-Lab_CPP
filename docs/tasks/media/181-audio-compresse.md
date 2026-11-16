# Lecture audio compressée

- [x] Décodage et document de bande
  - [x] Créer `src/MediaEngine/src/Formats/Common/Audio/DecodedPcm.h`, `DecodedPcm.cpp`, `Constants.h` : décodage Qt Multimedia depuis une source possédée, conversion PCM16LE, limites, annulation et nettoyage systématique.
  - [x] Créer `src/MediaEngine/src/Formats/Tape/Audio/Reader.h` et `Reader.cpp` : six extensions du C#, Vorbis paresseux pour OGG, segments intercalés et métadonnées de canal.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/audio-tape.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/audio-tape.json` : diagnostics traduisibles.
- [x] Dépendances et déploiement
  - [x] Modifier `CMakeLists.txt`, `src/MediaEngine/CMakeLists.txt`, `scripts/local-building/build.ps1`, `src/Launcher/LauncherConstants.h`, `src/Launcher/main.cpp` : Qt Multimedia, plugin audio et bibliothèques FFmpeg sous lib/FFmpeg.
  - [x] Modifier `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : constantes autorisées et solution.
- [x] Vérification ciblée
  - [x] Créer `build/verification-audio-tape/CMakeLists.txt` et `main.cpp` : document PCM multicanal, échec du décodage, annulation, fermeture des sources ; formats compressés selon fixtures disponibles.
  - [x] Supprimer `build/verification-audio-tape/` et les sorties temporaires après vérification.
