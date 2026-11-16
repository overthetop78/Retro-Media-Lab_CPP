# Détection des formats d'images

- [x] Contrats et règles
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/ImageFormatDetection.h` et `src/MediaEngine/src/Formats/Detection/Detector.h`, `Detector.cpp`, `Constants.h` : résultat possédé, confiance, choix utilisateur, contexte injectant l'ouverture, ordre des six règles et replis par extension.
  - [x] Créer `src/MediaEngine/src/Formats/Detection/Adf.cpp` et `Headers.cpp` : règles ADF/Acorn, Atari/MSA/ATX et Macintosh ; réutiliser les fonctions ADF partielles déjà portées, limiter les sondages et fermer les sources par propriété RAII.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/detection.json` et les fichiers de `Languages/<langue>/MediaEngine/detection.json` : copier les clés explicatives utilisées et leurs traductions existantes depuis `RetroMediaLab.App/Resources/<langue>/Detection.resx`, sans modifier le C#.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : intégrer les fichiers.
- [x] Vérification ciblée
  - [x] Créer `build/verification-format-detection/CMakeLists.txt` et `main.cpp` : priorité des règles et candidates, tailles reconnues, en-têtes minimaux/tronqués, erreurs IO absorbées et annulation propagée, résultat possédé et façade avec ouverture injectée.
  - [x] Supprimer `build/verification-format-detection/` et ses artefacts après utilisation.
