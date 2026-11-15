# Système de fichiers Macintosh HFS classique

- [x] Lecteur HFS et structures
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/MacHfs.h` et `src/MediaFileSystems/src/FileSystems/Apple/Macintosh/Hfs/Constants.h`, `Internal.h` : API sectorielle existante et structures privées de catalogue/extents, constantes reprises du C#, références aux helpers Macintosh communs.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Apple/Macintosh/Hfs/Extent.cpp`, `Catalog.cpp`, `Directory.cpp`, `Reader.cpp` : lecture des trois extents intégrés, secteurs absents positionnés, catalogue à nœuds feuilles, relations parent/enfant, forks distincts, métadonnées et diagnostics.
  - [x] Modifier les fichiers partagés MFS de compilation, diagnostics et audit et créer les catalogues HFS : intégration assurée par le parent ; réutiliser les primitifs Macintosh et les diagnostics contextuels déjà traduits.
- [x] Validation ciblée
  - [x] Créer `build/verification-mac-hfs/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer les véritables sources C# HFS, géométries et MDB invalides, extents incomplets, nœuds/clés/records limites, arborescence et cycles, forks données/ressources et annulation sans rejouer MFS.
  - [x] Supprimer `build/verification-mac-hfs/` et ses artefacts après validation.

- [x] Traductions HFS
  - [x] Modifier `src/Translations/Catalogs/Languages/*/MediaFileSystems/macintosh-hfs.json` : relire les 29 catalogues générés, corriger les six textes et préserver paramètres, HFS et Macintosh.

- [x] Constante de table d’offsets
  - [x] Modifier `src/MediaFileSystems/src/FileSystems/Apple/Macintosh/Hfs/Constants.h` et `Catalog.cpp` : nommer le décalage de deux entrées pour lire la borne de fin du record, sans changer sa valeur.
