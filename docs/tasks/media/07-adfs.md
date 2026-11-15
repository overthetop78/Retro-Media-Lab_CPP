# ADFS — exploration et initialisation des volumes

- [x] Lecture sectorielle et par blocs
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Adfs.h` : reconnaissance, lecture de volumes et production des en-têtes BBC/D, horodatages Acorn.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Acorn/Adfs/Constants.h`, `Internal.h`, `Reader.cpp`, `Directory.cpp`, `File.cpp`, `BlockView.cpp`, `Formatter.cpp`, `Time.cpp` : résolution FileCore, signatures Hugo/Nick, descendants, fichiers, dates et création des structures initiales ; vue de volume par blocs bornée et paresseuse.
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Acorn/Adfs/Constants.h` pour réutiliser les positions et tailles de carte déjà définies dans `../FileCore/Constants.h`, sans dupliquer leur définition.
- [x] Diagnostics et construction
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, `include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` et régénérer les projets Visual C++ : sources ADFS, constantes et diagnostics codés.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/adfs.json` et les équivalents dans `Languages/` : erreurs et avertissements ADFS traduits par Argos puis relus.
- [x] Validation
  - [x] Créer `build/verification-adfs/` : banc C#/C++ compilant les sources originales pour comparer volumes, fichiers, dates et en-têtes ; vérifier limites, intégrité, annulation et lecture par blocs.
  - [x] Supprimer `build/verification-adfs/` et ses artefacts après validation Debug et Release.
