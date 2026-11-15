# Collecte des candidats ISO depuis les pistes protégées

- [x] Sélection et collecte
  - [x] Créer `src/MediaEngine/src/Reconstruction/Iso/Collector.h`, `Collector.cpp`, `Score.cpp` et modifier `Constants.h` : score C# int32, choix stable avec NaN, décodeurs fournis par contrat, fenêtres et filtrage des secteurs, regroupements physiques/adressés possédés.
  - [x] Créer `src/MediaEngine/src/Reconstruction/Iso/Collection.cpp` : traitement parallèle borné, progression réelle, annulation/propagation des exceptions, fusion déterministe après jonction RAII.
- [x] Construction et erreurs
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` : nouvelles sources et diagnostics des contrats manquants.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/iso-collection.json` et ses 29 catalogues : erreurs de sélection vide et fonction de décodage absente, génération Argos puis relecture.
- [x] Validation
  - [x] Créer `build/verification-iso-collection/Reference.csproj`, `Reference.cs`, `generate.py`, `CMakeLists.txt`, `main.cpp` : comparaison du collecteur C# original avec décodeurs de contrôle, score/égalités/NaN, ordre des groupes et métadonnées, progression, absence de flux, exceptions et annulation ; intégration aux encodeurs/décodeurs FM/MFM ; langues et Debug/Release.
  - [x] Supprimer `build/verification-iso-collection/`, `build/iso-collection-reference.log`, `build/iso-collection-bench-debug.log`, `build/iso-collection-bench-release.log` : retrait des fichiers et artefacts du banc.

Le cache de fichiers et le registre général des décodeurs font l’objet de blocs suivants ; l’inventaire gardera IsoScpCandidateDecoder partiel tant que ces points restent absents. Les décodeurs sont fournis via un appel de service, correspondant à decoders.Decode dans la source.
