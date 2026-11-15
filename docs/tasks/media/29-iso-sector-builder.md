# Socle de construction sectorielle ISO

- [x] Candidats et mesure
  - [x] Créer `src/MediaEngine/src/Reconstruction/Iso/Models.h`, `Candidates.cpp`, `Builder.h`, `Measurement.cpp` : groupes ordonnés, candidats possédés et origine, sélection stable, données prioritaires non vides et mesure majoritaire exacte.
- [x] Construction et normalisation
  - [x] Créer `src/MediaEngine/src/Reconstruction/Iso/Builder.cpp`, `Normalizer.cpp`, `Constants.h` : image uniforme, filtres et fonctions d’index/normalisation, ordre physique enregistré, métadonnées source, opérations int32 bornées et padding sans troncature.
- [x] Géométrie déclarée et lecture
  - [x] Modifier `src/MediaEngine/src/Models/Sectors/SectorImages.cpp` : réserver la limite de taille d’une lecture aux sources adossées à des données ; autoriser une grande taille nominale dans une image sparse sans allocation de bloc.
- [x] Construction et diagnostics
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` ; créer `src/Translations/Catalogs/Constants/MediaEngine/iso-sector-builder.json` et les 29 catalogues : compilation, constantes déclarées, erreurs structurées/traduites et relecture Argos.
- [x] Validation
  - [x] Créer `build/verification-iso-sector-builder/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison aux sources C# originales pour majorité/égalités, meilleurs candidats, filtres/indexation, normalisation et métadonnées, refus, annulation et textes Debug/Release.
  - [x] Supprimer `build/verification-iso-sector-builder/`, `build/iso-sector-builder-reference.log`, `build/iso-sector-builder-bench-debug.log`, `build/iso-sector-builder-bench-release.log` : retrait des fichiers temporaires après vérification.
