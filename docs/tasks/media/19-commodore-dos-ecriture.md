# Écriture des volumes Commodore DOS

- [x] Contrats et allocation
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/CommodoreDos.h` et créer `src/MediaFileSystems/src/FileSystems/Commodore/Dos/WritePolicy.cpp`, `WriterInternal.h`, `WriteGeometry.cpp`, `Allocation.cpp` : politique validée, noms PETSCII, géométries standard, réservation et allocation fidèle au C#.
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Commodore/Dos/Constants.h` : offsets REL, identité DOS et constantes de construction des volumes.
- [x] Construction du volume
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Commodore/Dos/WriteFiles.cpp`, `WriteSystem.cpp`, `Writer.cpp` : chaînes de données, secteurs latéraux REL, répertoire, en-têtes/BAM et plan sectoriel possédé, avec annulation.
- [x] Construction et diagnostics
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, `include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` : compilation et diagnostics structurés du writer/politique.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/commodore-dos-writer.json` et les 29 catalogues de langue, modifier `scripts/translations/ignores/common.txt` : erreurs traduites et invariants REL.
- [x] Validation
  - [x] Créer `build/verification-commodore-dos-writer/Reference.csproj`, `Contracts.cs`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison aux sources C# originales des politiques, noms et octets/adresses des plans, limites de capacité/REL, erreurs, annulation et allers-retours lecture en Debug/Release.
  - [x] Supprimer `build/verification-commodore-dos-writer/`, `build/commodore-dos-writer-reference.log`, `build/commodore-dos-writer-bench-debug.log`, `build/commodore-dos-writer-bench-release.log` après validation.
