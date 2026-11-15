# Archives plates de ressources Amiga

- [x] Contrats et lecture
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/AmigaFlatArchive.h` : reconnaissance et projection de volume en lecture seule.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Amiga/FlatArchive/Constants.h`, `Internal.h`, `Directory.cpp`, `Data.cpp`, `Reader.cpp` : table Reserved, plages concaténées et anomalies physiques.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FileSystemModels.h` : référence traduisible facultative du commentaire technique, sans altérer les commentaires des fichiers.
  - [x] Modifier `src/MediaEngine/src/Formats/Adf/Reader.cpp` : sélectionner les archives plates reconnues après AmigaDOS, suivant le catalogue C#.
- [x] Construction et traduction
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, `include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` et régénérer les projets Visual C++ : compilation et diagnostics.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/amiga-flat.json` et les catalogues des 29 langues : erreurs, avertissements et commentaire traduisibles, génération Argos et relecture.
- [x] Validation
  - [x] Créer `build/verification-amiga-flat/` : comparer au C# original les tables, plages, secteurs courts/absents/invalides, contenus et diagnostics ; vérifier annulation et intégration ADF en Debug et Release.
  - [x] Supprimer `build/verification-amiga-flat/` et tous ses artefacts après validation.
