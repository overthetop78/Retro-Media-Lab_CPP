# Écriture D64/D71 et cartes de diagnostics

- [x] Writer et données
  - [x] Modifier `src/MediaEngine/src/Formats/CommodoreDisks/Images.h`, `Constants.h`, `Writer.cpp` : mode de conservation des diagnostics, codes sectoriels nommés, validation des géométries standard/étendues et écriture atomique en deux passes, sans déduire les codes de l’intégrité.
- [x] Diagnostics et traductions
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` : diagnostics structurés de géométrie/bloc/taille/diagnostic manquant.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/commodore-container-writer.json` et les 29 catalogues de langue : erreurs du writer, génération Argos et relecture.
- [x] Validation
  - [x] Créer `build/verification-commodore-container-writer/Reference.csproj`, `Contracts.cs`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison aux algorithmes C# originaux des payloads/cartes, cas invalides et ordre des erreurs ; conservation de destination sur échec/annulation, allers-retours lecture et volumes DOS en Debug/Release.
  - [x] Supprimer `build/verification-commodore-container-writer/`, `build/commodore-container-writer-reference.log`, `build/commodore-container-writer-bench-debug.log`, `build/commodore-container-writer-bench-release.log` après validation.
