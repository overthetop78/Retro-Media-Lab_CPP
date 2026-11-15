# DiskCopy 4.2

- [x] Conteneur et métadonnées
  - [x] Créer `src/MediaEngine/src/Formats/DiskCopy/Constants.h`, `DiskCopy.h`, `Reader.cpp` : porter les offsets, checksums, géométries taguées et lecture détaillée avec états d’intégrité du C#.
  - [x] Créer `src/MediaEngine/src/Formats/DiskCopy/Writer.cpp`, `Metadata.cpp` : porter les validations de charge utile, sérialisation, écriture atomique et métadonnées réversibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/diskcopy.json` et ses homologues `Languages/` : intégrer les trois sources, les constantes et les diagnostics traduits (intégration coordonnée par l’agent principal).
  - [x] Modifier `src/Translations/Catalogs/Languages/*/MediaEngine/diskcopy.json` : relire les 29 catalogues, corriger les termes techniques et préserver paramètres et noms invariants après génération Argos.
- [x] Validation ciblée
  - [x] Créer `build/verification-diskcopy/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer aux sources C# originales lecture, écriture, checksums, tags, métadonnées et refus ; contrôler les empreintes.
  - [x] Supprimer `build/verification-diskcopy/` et ses artefacts après validation.
