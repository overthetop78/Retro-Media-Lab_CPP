# Codec NorthStar MFM et secteurs matériels

- [x] Checksum commun et pistes NorthStar
  - [x] Créer `src/MediaEngine/src/Codecs/RotatingChecksum.h`, `RotatingChecksumConstants.h`, `RotatingChecksum.cpp` : XOR puis rotation sur huit bits, réutilisable par Heathkit.
  - [x] Créer `src/MediaEngine/src/Codecs/Northstar/Constants.h`, `Mfm.h`, `Address.cpp`, `Encoder.cpp`, `Decoder.cpp` : adresse par demi-octets, lecture identité/bloc, pistes et fenêtres de secteurs matériels FM/MFM ; résultats possédés et annulation.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compilation, diagnostics et constantes nommées.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/northstar-mfm.json` et ses 29 catalogues : tailles, coordonnées, fenêtres hors capacité et descriptions ; Argos puis relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger NorthStar.
- [x] Validation
  - [x] Créer `build/verification-northstar/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison avec sources originales pour checksum, adresses, pistes, champs directs et fenêtres FM/MFM, priorité des candidats, troncatures, limites, langues, registre et annulation Debug/Release.
  - [x] Supprimer `build/verification-northstar/` et les journaux associés après vérification : retrait du banc et de ses artefacts.
