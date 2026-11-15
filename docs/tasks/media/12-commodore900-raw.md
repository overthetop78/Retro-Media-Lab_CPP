# Commodore 900 — conteneur brut zoné COHERENT

- [x] Géométrie et image
  - [x] Créer `src/MediaEngine/src/Formats/Commodore900/Constants.h`, `CoherentImages.h`, `Geometry.cpp` : quatre zones, conversion bloc/adresse et bornes.
  - [x] Modifier `src/MediaEngine/src/Models/Sectors/SectorImages.h` et `SectorImages.cpp` : fournir aux images sur source une fonction d'adressage physique possédée, sans copie complète des secteurs.
- [x] Lecture, exploration et écriture
  - [x] Créer `src/MediaEngine/src/Formats/Commodore900/Reader.cpp` et `Writer.cpp` : reconnaissance/validation, image zonée de longueur réelle, exploration COHERENT/analyse, vérification des adresses et écriture atomique.
- [x] Construction et diagnostics
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` et régénérer les projets Visual C++ : compiler le conteneur et ses constantes/diagnostics.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/commodore900.json` et les catalogues des 29 langues : erreurs de géométrie, reconnaissance et écriture, génération Argos et relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger la marque et le modèle Commodore 900 dans les futures traductions.
- [x] Validation
  - [x] Créer `build/verification-commodore900/Reference.csproj`, `Adapters.cs`, `Reference.cs`, `generate.py`, `main.cpp` et `CMakeLists.txt` dans ce dossier : comparer la géométrie et les lectures/écritures aux cinq sources C# originales, tester reconnaissance partielle/corrompue, adresses de chaque secteur, annulation, préservation de destination et exploration COHERENT en Debug et Release.
  - [x] Supprimer `build/verification-commodore900/` et les journaux temporaires `build/commodore900-reference.log`, `build/commodore900-bench-debug.log`, `build/commodore900-bench-release.log` après validation.
