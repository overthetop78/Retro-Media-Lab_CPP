# BIN/CUE, sources audio et descripteurs

- [x] Structure du descripteur
  - [x] Créer `src/MediaEngine/src/Formats/Optical/BinCue/Constants.h`, `Sheet.h`, `Sheet.cpp` : fichiers, pistes, index, métadonnées, validation des collections et syntaxe CUE depuis le C#.
  - [x] Créer `src/MediaEngine/src/Formats/Optical/BinCue/SheetWriter.cpp` : sérialisation UTF-8, guillemets doublés, temps CD, ordre des index et métadonnées.
- [ ] Sources audio
  - [x] Créer `src/MediaEngine/src/Formats/Optical/BinCue/WaveConstants.h`, `WaveSource.h`, `WaveSource.cpp` : validation RIFF/WAVE et vue PCM stéréo 16 bits 44100 Hz, secteurs complets optionnels.
  - [ ] Créer `src/MediaEngine/src/Formats/Common/Audio/Vorbis/` : décodage Ogg/Vorbis seekable et vue PCM bornée, conforme au décodeur et à la source C# ; préciser les fichiers avant création.
- [ ] Lecture et écriture du jeu
  - [ ] Créer `src/MediaEngine/src/Formats/Optical/BinCue/Reader.h`, `Reader.cpp` : résolution BINARY/WAVE/OGG, pistes et index/pregaps, propriétés et reconnaissance.
  - [ ] Créer `src/MediaEngine/src/Formats/Optical/BinCue/Writer.cpp` : fichiers binaires par piste, pregaps stockés, CUE et publication atomique.
  - [ ] Modifier diagnostics, rejets et catalogues Constants/langues : erreurs CUE et WAVE traduisibles.
  - [ ] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : sources et constantes.
- [ ] Validation ciblée
  - [x] Créer `build/verification-bin-cue/CMakeLists.txt` et `main.cpp` : validation intermédiaire de la syntaxe/structure, métadonnées, sérialisation, WAVE PCM, bornes et annulation.
  - [x] Supprimer `build/verification-bin-cue/` et ses artefacts après consignation.

- [ ] Contrôle du raccordement final
  - [ ] Recréer `build/verification-bin-cue/CMakeLists.txt` et `main.cpp` après Vorbis et le raccordement des pistes : sources audio, données, index/pregaps, publication et annulation atomique.
  - [ ] Supprimer `build/verification-bin-cue/` et ses artefacts à la fin du contrôle final.
