# Modèle de pistes protégées et adaptateurs ISO

- [x] Modèle commun
  - [x] Créer `src/MediaEngine/src/Models/Flux/ProtectedTrack.h`, `ProtectedTrack.cpp`, `ProtectedTrackImage.cpp`, `ProtectedConstants.h` : révolutions avec résolution, plages de timing/structures/caractéristiques, propriété des données, validation et ordre des pistes/protection d’écriture source.
- [x] Adaptateurs
  - [x] Modifier `src/MediaEngine/src/Encoding/Track.h` et créer `Encoding/ProtectedImage.h`, `ProtectedImage.cpp` : piste encodée avec coordonnées/durée, conversion sans protection inventée, métadonnées temporelles et révolution préservées.
  - [x] Créer `src/MediaEngine/src/Formats/Scp/ProtectedImage.h`, `ProtectedImage.cpp` ; modifier `Scp.h`, `Windows.cpp` : capture SCP vers pistes protégées, résolution/protection conservées, fenêtres de décodage communes sans nouvelle interprétation des intervalles.
- [x] Construction et diagnostics
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` ; créer `src/Translations/Catalogs/Constants/MediaEngine/protected-tracks.json` et les 29 catalogues : compilation, constantes et erreurs traduites avec Argos puis relues.
- [x] Validation
  - [x] Créer `build/verification-protected-tracks/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison aux modèles/adaptateurs C# originaux, plages, timings, bits/révolutions, tri/doublons, flags SCP, données possédées, fenêtres, erreurs/annulation et langues Debug/Release.
  - [x] Supprimer `build/verification-protected-tracks/`, `build/protected-tracks-reference.log`, `build/protected-tracks-bench-debug.log`, `build/protected-tracks-bench-release.log` : retrait du banc et de ses artefacts après vérification.
