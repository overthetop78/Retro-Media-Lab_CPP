# Pistes Centurion MFM

- [x] Champs et pistes
  - [x] Créer `src/MediaEngine/src/Codecs/Centurion/Constants.h`, `Mfm.h`, `Fields.cpp` : identité CRC, allocation de blocs de 256 octets, préfixe et CRC de données avec validations de capacité.
  - [x] Créer `src/MediaEngine/src/Codecs/Centurion/Encoder.cpp`, `Decoder.cpp` : pistes MFM, recherche arrêtée au prochain en-tête, tailles déclarées, structures incomplètes et progression après bloc.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compiler les sources et déclarer erreurs/constantes nommées.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/centurion-mfm.json` et ses 29 catalogues : plages, CRC, clés non prises en charge et structures incomplètes ; Argos puis relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger Centurion.
- [x] Validation
  - [x] Créer `build/verification-centurion/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison C# des champs construits, allocations et limites, pistes/révolutions, clés/tailles, CRC, recherche et progression, structures non appariées, erreurs, langues, registre et annulation Debug/Release.
  - [x] Supprimer `build/verification-centurion/` et ses journaux après vérification : retrait du banc et de ses artefacts temporaires.
