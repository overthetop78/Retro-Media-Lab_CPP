# FileCore — cartes d'allocation Acorn

- [x] Contrats et décodage
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FileCore.h` : DiscRecord, adresses, résolveur possédant ses zones et primitives de bits vérifiées.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Acorn/FileCore/Constants.h`, `Codec.cpp`, `OldMap.cpp`, `NewMap.cpp` : champs/noms, cartes anciennes entrelacées et nouvelles cartes avec copie secondaire, sommes de contrôle, fragments et partage.
- [x] Construction et diagnostics
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, `Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` et régénérer les projets Visual C++ : compiler FileCore, constantes et erreurs codées.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/filecore.json` et ses équivalents sous `Languages/` : erreurs de bits, décalages et débordements ; les zones invalides sont refusées par la détection, Argos puis relecture.
- [x] Validation
  - [x] Créer `build/verification-filecore/` : référence compilant les sources C# originales et banc C++ comparant décodage, anciennes/nouvelles cartes, adresses et espace libre ; contrôles de bornes, corruption et annulation.
  - [x] Supprimer `build/verification-filecore/` et ses artefacts après validation Debug et Release.
