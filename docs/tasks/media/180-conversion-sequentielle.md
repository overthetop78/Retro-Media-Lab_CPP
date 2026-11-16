# Conversion séquentielle

- [x] Chaîne lecture, décodage et écriture
  - [x] Créer `src/MediaEngine/src/Images/Conversion/Sequential/Service.h`, `Service.cpp`, `Planning.cpp`, `Constants.h` : plan immuable, destinations, choix machine par intégrité, préservation des segments, pertes avant écriture, lecture des paramètres source et écriture par registre.
  - [x] Créer `src/MediaEngine/src/Images/Conversion/Sequential/SignalConverter.h` et `SignalConverter.cpp` : conversion UEF/Atom vers signal et PCM vers UEF, indépendante de l'analyse des fichiers.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h` et `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/sequential-conversion.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/sequential-conversion.json` : erreurs et pertes traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj`, `RetroMediaLab.vcxproj.filters` : intégrer la conversion.
- [x] Vérification ciblée du raccordement
  - [x] Modifier `scripts/quality/audit_literals.py` : déclarer le fichier des constantes de conversion dans les emplacements de constantes autorisés.
  - [x] Créer `build/verification-sequential-conversion/CMakeLists.txt` et `main.cpp` : sélection, plan, acceptation des pertes et écriture, sans relancer les suites de codecs.
  - [x] Supprimer `build/verification-sequential-conversion/` et ses sorties temporaires après utilisation.
