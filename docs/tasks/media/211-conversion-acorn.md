# Conversion des représentations Acorn

- [x] Conversion Acorn
  - [x] Créer `src/MediaEngine/src/Images/Conversion/Acorn.h`, `AcornRaw.cpp`, `AcornContainer.cpp`, `AcornConstants.h` : trois convertisseurs, contrôle des cartes et catalogues, validation des métadonnées, reformatage des blocs sans modification des octets, pertes explicites.
  - [x] Modifier `Composition/Conversion.cpp`, `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : inscrire les trois convertisseurs dans l'ordre C#.
  - [x] Modifier `Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`, créer `Catalogs/Constants/MediaEngine/acorn-conversion.json` et les catalogues de langues : diagnostics et avertissements traduits.
- [x] Validation du bloc
  - [x] Créer `build/verification-acorn-conversion/CMakeLists.txt` et `main.cpp` : conversion brute DFS/ADFS et conteneurs, données et métadonnées refusées, compiler le bloc.
  - [x] Supprimer `build/verification-acorn-conversion/` et ses artefacts après utilisation.
