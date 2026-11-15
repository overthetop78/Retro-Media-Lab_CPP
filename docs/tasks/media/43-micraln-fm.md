# Codec Micral N FM

- [x] Codec et somme de contrôle
  - [x] Créer `src/MediaEngine/src/Codecs/MicralN/Constants.h`, `Fm.h`, `Checksum.cpp`, `Encoder.cpp`, `Decoder.cpp` : préambule FM, secteurs de 128 octets, somme avec retenue, lecture de bloc et balayage fidèle au C#.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compilation et diagnostics/constantes nommés.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/micraln-fm.json` et ses 29 catalogues : erreurs et descriptions, génération Argos puis relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger le nom Micral N dans toutes les langues.
- [x] Validation
  - [x] Créer `build/verification-micraln/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : sources originales, matrice de retenue, séquences de checksum, pistes/blocs et corruptions, coordonnées/tailles, horloges, troncatures et marqueurs internes ; langues et annulation Debug/Release.
  - [x] Supprimer `build/verification-micraln/` et les journaux de référence/comparaison : retrait du banc et des artefacts après validation.
