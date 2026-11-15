# CP/M générique et Amstrad

- [x] Dispositions et contrats
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Cpm.h`, `src/MediaFileSystems/src/FileSystems/Cpm/Constants.h`, `Internal.h`, `Layout.cpp` : exposer les deux lecteurs et ensembles CatalogFormatIds exacts, porter les huit dispositions génériques, spécification PCW et résolution Epson/CPC/PCW sans recopier les algorithmes de répertoire.
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/CpmLayoutConstants.h` et modifier `Cpm.h`, `FileSystems/Cpm/Constants.h` : exposer les constantes publiques C# CPC/PCW nécessaires au contrat Engine/SDK et aliaser les valeurs privées sans duplication ni changement de valeur.
- [x] Répertoire et fichiers
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Cpm/Directory.cpp`, `Content.cpp`, `Reader.cpp` : porter aplatissement positionnel, disponibilité, reconnaissance/scoring/recherche des répertoires, noms8.3/attributs/labels, regroupement d’extents, allocations et reconstruction avec les modèles MFS existants ; mutualiser le volume entre lecteurs générique et Amstrad sans changer leurs règles.
  - [x] Modifier les fichiers partagés de compilation, diagnostics, audit et `FormatIds.h` et créer les catalogues `Constants/MediaFileSystems/cpm.json`, `Languages/*/MediaFileSystems/cpm.json` : intégrer les identifiants exacts et diagnostics traduits fournis au parent (agent parent).
  - [x] Modifier les catalogues `src/Translations/Catalogs/Languages/*/MediaFileSystems/cpm.json` : relire/corriger les traductions Argos et vérifier paramètres/invariants selon partage coordonné avec le parent.
- [x] Vérification ciblée
  - [x] Créer `build/verification-cpm/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer les 12 sources C# originales et les contrats de volumes sur dispositions, PCW/CPC/Epson, scoring, extents, fichiers et défauts structurels ; vérifier annulation C++ sans rejouer les blocs précédents.
  - [x] Supprimer `build/verification-cpm/` et les artefacts après validation et intégration de l’inventaire par le parent.
