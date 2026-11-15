# Système de fichiers Apple DOS

- [x] Lecture et écriture
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/AppleDos.h`, `src/MediaFileSystems/src/FileSystems/Apple/Dos/Constants.h`, `Internal.h`, `Codec.cpp`, `Reader.cpp`, `Writer.cpp` : porter VTOC, catalogues, chaînes T/S, noms, types, fichiers binaires et création de volumes DOS 3.2/3.3 avec les contrats MigrationPlan/SectorWritePlan existants.
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, `scripts/quality/audit_literals.py`, les diagnostics MFS et créer `src/Translations/Catalogs/Constants/MediaFileSystems/apple-dos.json` avec les homologues `Languages/` : intégrer les sources et traductions (fichiers partagés pris en charge par l’agent parent).
- [x] Vérification ciblée
  - [x] Créer `build/verification-apple-dos/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer les sources C# originales sur les volumes créés, fichiers lus, chaînes invalides et politiques de noms ; compiler les sources nouvelles et vérifier les diagnostics.
  - [x] Supprimer `build/verification-apple-dos/` et les journaux du banc après validation.
