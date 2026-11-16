# Session de lecture PCM

- [x] Lecture et durée de vie
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/PcmPlaybackSession.h` et `src/MediaEngine/src/Audio/Playback.cpp` : session possédée, ouverture via reconnaissance audio, lecture signée 16 bits entrelacée, position par trame, déplacement et fermeture RAII fidèle au C#.
  - [x] Créer `src/MediaEngine/src/Audio/PlaybackConstants.h` : taille du tampon et constantes techniques propres à la session.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : intégrer la session.
- [x] Vérification ciblée
  - [x] Créer `build/verification-pcm-playback/CMakeLists.txt` et `main.cpp` : lectures multicanales fragmentées, position, fin de flux, annulation, refus de métadonnées invalides et libération sur erreur.
  - [x] Supprimer `build/verification-pcm-playback/` et ses artefacts après utilisation.
