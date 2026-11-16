# Catalogue principal des formats d'images

- [x] Contrats et données
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/ImageFormat.h` et `src/MediaEngine/src/Formats/Catalog/Catalogue.h`, `Catalogue.cpp`, `Constants.h` : contrats de format/extension, dimensions physiques, densité, compatibilité et accès au catalogue embarqué avec textes référencés par clés.
  - [x] Créer temporairement `build/port-image-format-catalogue/` avec projet/exporteur C# : exporter les données du constructeur original et ses constantes dans un fichier JSON, sans modifier ni reconstruire le projet source.
  - [x] Créer `src/MediaEngine/src/Formats/Catalog/Data/formats.json` : conserver toutes les entrées et propriétés exportées, remplacer les libellés par leurs clés de traduction.
  - [x] Créer les catalogues `src/Translations/Catalogs/Constants/MediaEngine/formats.json`, `extensions.json`, `families.json` et leurs équivalents `Languages/<langue>/MediaEngine/` : reprendre les textes/traductions C# disponibles, protéger les invariants et traduire les nouveaux textes avec Argos si nécessaire.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger les noms techniques du catalogue lorsqu'ils apparaissent dans une description traduite.
  - [x] Modifier les catalogues signalés sous `Languages/{ru-RU,pt-PT,pt-BR,ar-SA,th-TH,vi-VN}/MediaEngine/amstrad-fds.json`, `Languages/de-DE/MediaEngine/apple-geometry.json`, `Languages/{ar-SA,th-TH}/MediaFileSystems/cartridge-readers.json` et `Languages/{ja-JP,ar-SA,th-TH}/MediaFileSystems/sector-registry.json` : rétablir les invariants ROM, ADM, Apple Macintosh et Atari altérés dans les textes concernés.
- [x] Intégration
  - [x] Modifier `src/MediaEngine/src/Formats/Definitions/DiskDefinitions.h`, `Reader.cpp`, `RawImages.h` et `RawImages.cpp` : brancher par défaut les définitions brutes et leurs adapters sur la liste complète des formats réservés.
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`) et créer les catalogues `Constants/MediaEngine/format-catalog-errors.json` et leurs équivalents par langue : signaler les données embarquées invalides avec une erreur traduite.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` et `scripts/quality/audit_literals.py` : compiler et embarquer le catalogue, déclarer son fichier de constantes.
- [x] Contrôle ciblé et nettoyage
  - [x] Créer `build/verification-image-format-catalogue/CMakeLists.txt` et `main.cpp` : vérifier le chargement de toutes les entrées exportées, la compatibilité et l'exclusion complète des formats réservés dans les adapters bruts par défaut.
  - [x] Supprimer `build/port-image-format-catalogue/` et `build/verification-image-format-catalogue/` avec leurs binaires et fichiers temporaires après consignation.
