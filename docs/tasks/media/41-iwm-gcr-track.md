# Pistes IWM Macintosh et Lisa

- [x] Codecs de piste communs
  - [x] Modifier `src/MediaEngine/src/Codecs/AppleIwm/Constants.h` et créer `Gcr.h`, `Encoder.cpp`, `Decoder.cpp` : identités Macintosh/Lisa, formats et tags, adresses, blocs de données, checksum, intégrités distinctes et structures triées ; décodage NRZI automatique ou à cellule imposée.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` : compilation et erreurs sectorielles/de plage nommées.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/iwm-gcr-track.json` et ses 29 catalogues : erreurs et descriptions localisées ; génération Argos puis relecture, réutilisation des formulations Apple déjà corrigées.
- [x] Validation
  - [x] Créer `build/verification-iwm-track/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : sources C# originales, deux identités, tags/formats, coordonnées, erreurs, champs tronqués/corrompus, limites de recherche, rebouclage, NRZI imposé/automatique, langues et annulation Debug/Release.
  - [x] Supprimer `build/verification-iwm-track/` et les journaux de comparaison : retrait du banc et de ses artefacts après vérification.
