# Apple Lisa Office — systèmes de fichiers

- [x] Lecteur et primitives Lisa
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Lisa.h`, `src/MediaFileSystems/src/FileSystems/Apple/Lisa/Constants.h`, `Internal.h`, `VolumeHeader.cpp` : porter les versions/noms techniques de catalogue et réutiliser les en-têtes/noms Latin1 déjà portés dans AppleRecognition, sans dupliquer leurs algorithmes.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Apple/Lisa/Tags.cpp`, `Catalog.cpp`, `Content.cpp`, `Reader.cpp` : porter tags12 octets, MDDF, catalogues tabulaire/haché/B-tree, regroupement/ordonnancement des pages, lacunes/doublons et volumes avec les contrats MFS existants.
  - [x] Modifier les fichiers partagés `src/MediaFileSystems/CMakeLists.txt`, diagnostics et audit de constantes et créer les catalogues `Constants/MediaFileSystems/lisa.json` et `Languages/*/MediaFileSystems/lisa.json` : intégrer les sources et diagnostics structurés traduits (agent parent).
  - [x] Modifier les catalogues de langues `src/Translations/Catalogs/Languages/*/MediaFileSystems/lisa.json` : relire les traductions Argos, corriger les erreurs et contrôler les paramètres/invariants selon partage de langues coordonné par le parent.
- [x] Vérification ciblée
  - [x] Créer `build/verification-lisa/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer les 12 fichiers C# originaux sur tags, MDDF, noms/versions, catalogues, pages absentes/dupliquées, métadonnées et erreurs, plus annulation C++, sans relancer les blocs précédents.
  - [x] Supprimer `build/verification-lisa/` et ses artefacts après validation et intégration des empreintes dans l’inventaire par le parent.
