# Géométries Commodore et conteneurs D64/D71/D81

- [x] Géométries partagées
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/CommodoreGeometry.h`, `CommodoreGeometryConstants.h` et `src/MediaFileSystems/src/Formats/Commodore/Geometry.cpp` : zones/adresses 1541, deux faces 1571, vues logique/physique 1581 et validations fidèles au C#.
- [x] Conteneurs
  - [x] Créer `src/MediaEngine/src/Formats/CommodoreDisks/Images.h`, `Constants.h`, `Layouts.cpp`, `Reader.cpp`, `Writer.cpp` : catalogues de dispositions, reconnaissance par longueur/format, sources possédées et lecture différée des secteurs avec carte d’erreurs, D81 logique et écriture atomique D81.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `src/MediaFileSystems/CMakeLists.txt`, les `Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` de ces bibliothèques et `scripts/quality/audit_literals.py` : compilation, diagnostics et constantes nommées ; régénérer les projets Visual C++.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/commodore-geometry.json`, `Constants/MediaEngine/commodore-disks.json` et les 29 catalogues correspondants : erreurs et noms de dispositions traduisibles, génération Argos et relecture.
- [x] Validation
  - [x] Créer `build/verification-commodore-disks/Reference.csproj`, `Reference.cs`, `Contracts.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison aux sources C# originales des géométries/adresses, dispositions, blocs/payloads/cartes d’erreurs, lecture différée, reconnaissance et D81/atomicité/annulation en Debug et Release.
  - [x] Supprimer `build/verification-commodore-disks/`, `build/commodore-disks-reference.log`, `build/commodore-disks-bench-debug.log`, `build/commodore-disks-bench-release.log` après validation.
