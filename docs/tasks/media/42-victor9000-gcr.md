# Codec Victor 9000 GCR

- [x] Champs, checksum et pistes
  - [x] Créer `src/MediaEngine/src/Codecs/Victor9000/Constants.h`, `Gcr.h`, `Fields.cpp`, `Encoder.cpp`, `Decoder.cpp` : marques entrelacées, somme d’adresse et somme 16 bits, encodage partagé Commodore, lecture des champs et données appariées/non appariées selon le C#.
- [x] Construction et diagnostics
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compilation et diagnostics/constantes nommés.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/victor9000-gcr.json` et ses 29 catalogues : erreurs et descriptions localisées, génération Argos puis relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger le nom Victor 9000 dans toutes les langues.
- [x] Validation
  - [x] Créer `build/verification-victor9000/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : sources originales, champs et checksums, marqueurs limites, pistes/flux et corruptions, intégrités et métadonnées, langues et annulation Debug/Release.
  - [x] Supprimer `build/verification-victor9000/` et les journaux de référence/comparaison : retrait du banc et de ses artefacts après vérification.
