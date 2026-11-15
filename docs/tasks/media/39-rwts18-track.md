# Pistes Apple RWTS18

- [x] Encodeur et décodeur de piste
  - [x] Modifier `src/MediaEngine/src/Codecs/AppleRwts18/Constants.h` et créer `Gcr.h`, `Encoder.cpp`, `Decoder.cpp` : six secteurs uniques 0–5, ordre décroissant, adresses et identifiant, lecture synchronisée, fenêtre de données, intégrité et positions circulaires selon le C#.
- [x] Construction et diagnostics
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` : compilation et diagnostics nommés de disposition/plage.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/rwts18-track.json` et les 29 catalogues : erreurs et descriptions de structures, génération Argos puis relecture.
- [x] Validation
  - [x] Créer `build/verification-rwts18-track/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : sources originales, ordre et contenu, coordonnées/identifiants, dispositions erronées, troncatures/corruptions, décalages de synchronisation et rebouclage, langues et annulation Debug/Release.
  - [x] Supprimer `build/verification-rwts18-track/` et les journaux de comparaison : retrait du banc et de ses artefacts après vérification.
