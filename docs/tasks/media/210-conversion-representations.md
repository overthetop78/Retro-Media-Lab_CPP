# Conversion des représentations

- [x] Contrats et conversions
  - [x] Créer `src/MediaEngine/src/Images/Conversion/Representation.h` et `Representation.cpp` : contrat, résultat possédé, registre déterministe et validation.
  - [x] Créer `Images/Conversion/Constants.h`, `FluxSectors.h`, `FluxSectors.cpp`, `HfeCells.cpp` : flux vers secteurs, secteurs vers flux et cellules HFE, progression et pertes déclarées.
  - [x] Créer `Composition/Conversion.h`, `Conversion.cpp`, modifier `Composition/Runtime.h` : composition partagée, formats cibles repris des lecteurs et définitions, adaptation du convertisseur de signal existant.
  - [x] Modifier `Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`, créer `Catalogs/Constants/MediaEngine/representation-conversion.json` et ses catalogues de langues : erreurs, progression et pertes traduites.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : intégrer les nouvelles sources et catalogues.
- [x] Validation du bloc
  - [x] Créer `build/verification-representation-conversion/CMakeLists.txt` et `main.cpp` : registre, aller-retour de représentation, options de conteneur, cellules HFE et pertes ; compiler le bloc.
  - [x] Supprimer `build/verification-representation-conversion/` et ses artefacts après utilisation.
