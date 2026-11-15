# Reconstruction SCP Commodore 1581

- [x] Accumulation et lecture
  - [x] Créer `src/MediaEngine/src/Formats/CommodoreDisks/MfmReconstruction.h`, `MfmReconstruction.cpp` : candidats par coordonnées physiques, priorité d’intégrité stable, découpage 512 vers 256 octets, inversion des faces et erreurs distinctes de capture vide/candidats inutilisables.
  - [x] Modifier `src/MediaEngine/src/Formats/CommodoreDisks/FluxReader.h`, `FluxReader.cpp`, `src/MediaEngine/CMakeLists.txt` : parcours des fenêtres SCP avec décodeur ISO MFM, progression et annulation, compilation du reconstructeur.
- [x] Diagnostics
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, modifier `scripts/translations/ignores/common.txt` et créer `src/Translations/Catalogs/Constants/MediaEngine/commodore-scp-mfm.json`, les 29 catalogues correspondants : diagnostics de reconstruction 1581, génération Argos et relecture.
- [x] Validation
  - [x] Créer `build/verification-commodore-scp-mfm/Reference.csproj`, `Contracts.cs`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison à la branche C# originale, limites physiques/logiques, priorité et stabilité, données/métadonnées, flux MFM réels et progression, annulation, traductions Debug/Release.
  - [x] Supprimer `build/verification-commodore-scp-mfm/`, `build/commodore-scp-mfm-reference.log`, `build/commodore-scp-mfm-bench-debug.log`, `build/commodore-scp-mfm-bench-release.log` : retrait des fichiers temporaires après vérification.
