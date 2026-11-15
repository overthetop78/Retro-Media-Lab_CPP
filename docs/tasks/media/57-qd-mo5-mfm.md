# Pistes QD MO5 MFM

- [x] Champs et pistes
  - [x] Créer `src/MediaEngine/src/Codecs/QdMo5/Constants.h`, `Mfm.h`, `Fields.cpp` : préambule, numéro de secteur grand-boutiste, checksum additif et helpers de lecture/recherche bornée.
  - [x] Créer `src/MediaEngine/src/Codecs/QdMo5/Encoder.cpp`, `Decoder.cpp` : préfixe sectoriel, zone réservée, appairage arrêté au prochain en-tête, intégrité et tri stable des structures.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : sources regroupées, diagnostics et constantes nommées.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger QD MO5.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/qd-mo5-mfm.json` et ses 29 catalogues : plages, taille, checksum, préfixe et structures ; Argos puis relecture.
- [x] Validation
  - [x] Créer `build/verification-qd-mo5/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparer les sources C# originales, secteurs 16 bits, préfixes, checksum, champs, limites de recherche, blocs incomplets, pistes multiples, diagnostics, registre, annulation et langues Debug/Release.
  - [x] Supprimer `build/verification-qd-mo5/` et ses journaux après vérification : retrait du banc et de ses artefacts temporaires.
