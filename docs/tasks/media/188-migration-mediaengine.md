# Service de migration MediaEngine

- [x] API et composition
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/Migration.h`, `src/Images/Creation/Constants.h` et `Catalog.cpp` : onze destinations du C#, pertes et rapport publics indépendants de MediaFileSystems.
  - [x] Créer `src/MediaEngine/src/Images/Creation/Blank.h` et `Blank.cpp` : images vierges du C#, profils sectoriels existants, catalogues Acorn et géométries zonées Apple/Commodore.
  - [x] Créer `src/MediaEngine/src/Images/Creation/Migration.cpp` : préparer les seules données consommées par le planificateur, valider/injecter via MediaFileSystems, retourner un document opaque et écrire les conteneurs C# en tâche asynchrone.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h` et `src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/migration.json` et `Languages/*/MediaEngine/migration.json` : erreurs de sélection et de conteneur traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-migration-engine/CMakeLists.txt` et `main.cpp` : onze destinations, création et écriture/relecture, refus des conteneurs invalides et pertes non acceptées, indépendance des données et annulation ; images vierges Acorn et géométries zonées.
  - [x] Supprimer `build/verification-migration-engine/` et ses sorties après utilisation.
