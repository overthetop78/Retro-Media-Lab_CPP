# Reconstruction SCP Commodore GCR

- [x] Sélection et géométrie
  - [x] Créer `src/MediaEngine/src/Formats/CommodoreDisks/GcrReconstruction.h`, `GcrReconstruction.cpp` : sélection stable des candidats, conservation de leur ordre, détection 1541/1571/étendue depuis tous les candidats et construction sectorielle sans remplissage des manques.
- [x] Lecture des captures
  - [x] Créer `src/MediaEngine/src/Formats/CommodoreDisks/FluxReader.h`, `FluxReader.cpp` : parcours des fenêtres SCP, décodage GCR commun, progression par piste, annulation et raccordement du résultat à l’image sectorielle.
- [x] Construction et diagnostics
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` : compilation et erreur structurée lorsqu’aucun candidat GCR ne peut être décodé.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/commodore-scp-gcr.json` et les 29 catalogues de langue : diagnostic traduit, génération Argos et relecture.
- [x] Validation
  - [x] Créer `build/verification-commodore-scp-gcr/Reference.csproj`, `Reference.cs`, `Contracts.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison au reconstructeur C# original pour ordre des candidats, priorités, inférence géométrique, filtres, révolutions/progression, captures GCR/SCP synthétiques et annulation en Debug/Release.
  - [x] Supprimer `build/verification-commodore-scp-gcr/`, `build/commodore-scp-gcr-reference.log`, `build/commodore-scp-gcr-bench-debug.log`, `build/commodore-scp-gcr-bench-release.log` après validation.
