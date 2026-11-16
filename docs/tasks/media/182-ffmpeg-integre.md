# FFmpeg intégré et conversion audio

- [x] Distribution autonome Windows
  - [x] Créer `third_party/ffmpeg/package.json` et `scripts/local-building/prepare-ffmpeg.ps1` : version et empreintes fixées, cache vérifié, copie depuis une installation fournie ou téléchargement contrôlé, licence et provenance.
  - [x] Modifier `scripts/local-building/build.ps1` : embarquer FFmpeg dans `lib/FFmpeg` avec sa licence, sans dépendance au PATH du PC utilisateur.
- [x] Conversion audio réutilisable
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/AudioConversion.h`, `src/MediaEngine/src/Audio/Conversion.cpp` et `Constants.h` : WAV, MP3, OGG, FLAC, AAC, M4A, WMA ; paramètres validés, processus sans shell, annulation et protection des fichiers de sortie.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/audio-conversion.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/audio-conversion.json` : erreurs traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `scripts/translations/ignores/common.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compilation et constantes.
- [x] Vérification ciblée
  - [x] Créer `build/verification-ffmpeg-audio/CMakeLists.txt` et `main.cpp` : conversions WAV vers compression, compression vers WAV et entre compressions, conservation des sorties en erreur, annulation et chemin embarqué.
  - [x] Supprimer `build/verification-ffmpeg-audio/` et les sorties temporaires après utilisation.
  - [x] Créer temporairement `build/Debug/Retro Media Lab/verify-audio.exe` par copie du contrôle compilé : exécuter une conversion avec le service par défaut et les seules DLL de la distribution, sans Qt ni FFmpeg du PATH hôte.
  - [x] Supprimer `build/Debug/Retro Media Lab/verify-audio.exe` après le contrôle de distribution.
  - [x] Supprimer `build/tools/ffmpeg/` : enlever la première copie de validation, remplacée par le cache de dépendance vérifié.
