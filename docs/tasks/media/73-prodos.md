# Système de fichiers Apple ProDOS/SOS

- [x] Lecture et structures
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/ProDos.h`, `src/MediaFileSystems/src/FileSystems/Apple/ProDos/Constants.h`, `Internal.h`, `Codec.cpp` : contrats existants, types de stockage, noms, primitives binaires, dates et en-tête de volume, réutilisation de la validation Apple commune.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Apple/ProDos/Reader.cpp`, `Directory.cpp`, `File.cpp`, `Bitmap.cpp` : parcours bornés des répertoires, fichiers seedling/sapling/tree et plages creuses, avertissements et calcul d’espace libre.
- [x] Création de volumes
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Apple/ProDos/Writer.cpp` : allocation, répertoires, index, bitmap et SectorWritePlan depuis MigrationPlan, avec horodatage déterministe optionnel et nettoyage RAII.
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, les diagnostics MFS et `scripts/quality/audit_literals.py`, créer les catalogues `Constants/MediaFileSystems/prodos.json` et `Languages/*/MediaFileSystems/prodos.json` : intégration partagée et traductions assurées par le parent.
  - [x] Modifier `src/Translations/Catalogs/Languages/*/MediaFileSystems/prodos.json` : relire et corriger les 29 catalogues générés, avec répartition des langues entre parent et agent ProDOS, préserver types techniques et paramètres.
- [x] Vérification ciblée
  - [x] Créer `build/verification-prodos/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : compiler les véritables sources C#, comparer métadonnées, contenus, blocs écrits, trous, liens/cycles, bitmaps, noms et dates limites sans relancer les tests DOS/2IMG déjà acquis.
  - [x] Supprimer `build/verification-prodos/` et les journaux temporaires après validation.

- [x] Diagnostic précis de l’extension Timestamp C++
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Apple/ProDos/Codec.cpp` : remplacer uniquement le diagnostic de date invalide par `ProDosTimestamp` ; intégration partagée de la clé et du catalogue `prodos-timestamp.json` par le parent.
  - [x] Modifier `build/verification-prodos/main.cpp` : ajouter un mode ciblé vérifiant le diagnostic de date invalide, après la prochaine compilation groupée, sans rejouer les 150 comparaisons acquises.
