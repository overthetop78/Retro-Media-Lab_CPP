# Codec de piste Apple II GCR

- [x] Codec et constantes de piste
  - [x] Modifier `src/MediaEngine/src/Codecs/AppleII/Constants.h` et créer `Gcr.h`, `Encoder.cpp`, `Decoder.cpp` : paramètres 13/16 secteurs, attributs explicites, adresses, association des données, rebouclage circulaire et descriptions localisées fidèles au C#.
- [x] Compilation et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` : compilation et erreurs de format/adresse nommées.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/appleii-gcr-track.json` et les 29 catalogues de langues : diagnostics et descriptions de structures, génération Argos et relecture.
- [x] Validation
  - [x] Créer `build/verification-appleii-track/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison des sources originales pour pistes 13/16 secteurs, formats explicites/implicites, limites d’adresses, secteurs incorrects, structures tronquées/corrompues/non appariées, rebouclage, métadonnées, intégrité et langues Debug/Release.
  - [x] Supprimer `build/verification-appleii-track/` et les journaux de comparaison associés : retrait du banc, binaires et artefacts temporaires après vérification.
