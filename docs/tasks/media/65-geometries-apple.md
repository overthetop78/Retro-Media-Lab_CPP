# Géométries et dispositions Apple

- [x] Géométries communes aux images et à la reconstruction
  - [x] Créer `src/MediaEngine/src/Formats/Apple/GeometryConstants.h`, `Geometry.h`, `Geometry.cpp` : reprendre les géométries Apple II, Macintosh GCR/MFM, Lisa FileWare et Apple tagué, les permutations et le catalogue des capacités ambiguës.
  - [x] Modifier `src/MediaEngine/src/Reconstruction/Sector/Apple.cpp`, `Constants.h` : réutiliser les constantes et zones Apple communes sans modifier les règles de sélection ou les diagnostics existants.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/apple-geometry.json` et ses homologues dans `Languages/` : intégrer la compilation et les erreurs traduites des géométries.
- [x] Vérification
  - [x] Créer `build/verification-apple-geometry/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt` : comparer les classes C# originales sur tous les blocs et cylindres valides, les limites invalides, les permutations et toutes les capacités cataloguées ; compiler Debug/Release et vérifier les frontières et littéraux.
  - [x] Supprimer `build/verification-apple-geometry/` et les journaux temporaires après validation.
