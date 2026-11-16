# Préparation et validation des migrations

- [x] Contrats et règles du service C#
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Migration.h` : capacités de destination, pertes structurées, rapport, politique de noms et API de préparation/validation/réduction.
  - [x] Créer `src/MediaFileSystems/src/Migration/Constants.h` et `Planning.cpp` : chemins source, validation des noms et limites, collisions par dossier, distinction des pertes bloquantes et métadonnées, réduction explicite.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h` et `src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaFileSystems/migration.json` et `Languages/*/MediaFileSystems/migration.json` : erreurs traduisibles de préparation et validation.
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-migration-validation/CMakeLists.txt` et `main.cpp` : copie indépendante, chemins imbriqués, noms Unicode, collisions, pertes bloquantes, consentement et réduction des seules métadonnées non prises en charge.
  - [x] Supprimer `build/verification-migration-validation/` et ses sorties après utilisation.
- [x] Classement des erreurs
  - [x] Modifier `src/MediaFileSystems/src/Diagnostics.cpp` : attribuer explicitement InvalidArgument aux identifiants invalides, ArgumentOutOfRange aux limites invalides et Other au refus d'exécution.
