# BIN/CUE, sources audio et descripteurs

- [x] Structure du descripteur
  - [x] Créer `src/MediaEngine/src/Formats/Optical/BinCue/Constants.h`, `Sheet.h`, `Sheet.cpp` : fichiers, pistes, index, métadonnées, validation des collections et syntaxe CUE depuis le C#.
  - [x] Créer `src/MediaEngine/src/Formats/Optical/BinCue/SheetWriter.cpp` : sérialisation UTF-8, guillemets doublés, temps CD, ordre des index et métadonnées.
- [x] Sources audio
  - [x] Créer `src/MediaEngine/src/Formats/Optical/BinCue/WaveConstants.h`, `WaveSource.h`, `WaveSource.cpp` : validation RIFF/WAVE et vue PCM stéréo 16 bits 44100 Hz, secteurs complets optionnels.
  - [x] Créer `src/MediaEngine/src/Formats/Common/Audio/Vorbis/Constants.h`, `PcmSource.h`, `PcmSource.cpp` : source PCM Tremor possédée, seek, callbacks bornés sans exception traversant le C, annulation, chaînes au même profil, PCM little-endian et lectures non alignées.
- [x] Lecture et écriture du jeu
  - [x] Créer `src/MediaEngine/src/Formats/Optical/BinCue/Reader.h`, `Reader.cpp` : résolution BINARY/WAVE/OGG, pistes et index/pregaps, propriétés et reconnaissance.
  - [x] Créer `src/MediaEngine/src/Formats/Optical/BinCue/Writer.cpp` : fichiers binaires par piste, pregaps stockés, CUE et publication atomique.
  - [x] Modifier diagnostics, rejets et catalogues Constants/langues : erreurs CUE et WAVE traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : sources et constantes.
- [x] Validation ciblée
  - [x] Créer `build/verification-bin-cue/CMakeLists.txt` et `main.cpp` : validation intermédiaire de la syntaxe/structure, métadonnées, sérialisation, WAVE PCM, bornes et annulation.
  - [x] Supprimer `build/verification-bin-cue/` et ses artefacts après consignation.

- [x] Contrôle du raccordement final
  - [x] Recréer `build/verification-bin-cue/CMakeLists.txt` et `main.cpp` après Vorbis et le raccordement des pistes : sources audio, données, index/pregaps, publication et annulation atomique.
  - [x] Supprimer `build/verification-bin-cue/` et ses artefacts à la fin du contrôle final.

- [x] Dépendances natives Vorbis
  - [x] Créer `build/vendor-vorbis/tremor/` depuis le dépôt officiel Xiph, révision `820fb3237ea81af44c9cc468c8b4e20128e3e5ad`, et `build/vendor-vorbis/libogg-1.3.6.tar.xz` depuis l'archive officielle ; vérifier son SHA-256 avant extraction.
  - [x] Créer `third_party/tremor/` et `third_party/ogg/` : sources et en-têtes utiles, licences, fichiers CMake de compilation statique portable ; retenir les 13 `.c` de `Makefile.am`, les 17 `.h` racine et `COPYING` de Tremor ; pour Ogg, `src/bitwise.c`, `src/framing.c`, `src/crctable.h`, `include/ogg/ogg.h`, `os_types.h`, `config_types.h` et `COPYING` ; ajouter un `CMakeLists.txt` dans chaque dossier ; supprimer le qualificatif const redondant des tables `mdct_lookup.h` et `window_lookup.h`, charger stdlib avant les macros min/max sous MSVC.
  - [x] Modifier `third_party/CMakeLists.txt` : compilation et installation des licences des deux dépendances intégrées, sans téléchargement au build.
  - [x] Supprimer `build/vendor-vorbis/` après intégration des sources utiles.
- [x] Intégration de la source Vorbis
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, `src/MediaEngine/src/Images/ReadRejection.cpp` : diagnostics Vorbis traduisibles.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/vorbis.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/vorbis.json` : erreurs de données, profil, bornes et décodage/seek.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : source PCM, lien Tremor et constantes.
- [x] Comparaison ciblée Vorbis
  - [x] Créer `build/verification-bin-cue/CMakeLists.txt`, `main.cpp`, `reference/reference.csproj` et `reference/Program.cs` : référence C# en lecture seule et comparaison PCM native avec chemins Ogg fournis en arguments ; lectures aléatoires non alignées, annulation et reprises après erreur.
  - [x] Supprimer `build/verification-bin-cue/` avec les sorties PCM et binaires temporaires après consignation.

- [x] Modifier `scripts/translations/ignores/common.txt` : protéger les noms Ogg et Vorbis pour toutes les langues.
- [x] Modifier `scripts/local-building/update-solution.py`, `RetroMediaLab.vcxproj` et `.filters` : inclure les en-têtes Tremor/Ogg dans la navigation Visual C++.
