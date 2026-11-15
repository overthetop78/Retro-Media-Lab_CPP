# Codec Micropolis MFM et records

- [x] Records et pistes Micropolis
  - [x] Créer `src/MediaEngine/src/Codecs/Micropolis/Constants.h`, `Mfm.h`, `Record.cpp`, `Encoder.cpp`, `Decoder.cpp` : checksum à réduction différée, records complets possédés, parse/création, modes données seules/record, synchronisation et balayage fidèles au C#.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compilation et diagnostics/constantes nommés.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/micropolis-mfm.json` et ses 29 catalogues : erreurs de taille/numéro et de capacité du record, descriptions ; génération Argos puis relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger Micropolis dans toutes les langues.
- [x] Validation
  - [x] Créer `build/verification-micropolis/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : sources originales, seuils de checksum, création/parse/mutation des records, réserves/trailer, pistes et modes de sortie, corruption/troncature, horloges, langues et annulation Debug/Release.
  - [x] Supprimer `build/verification-micropolis/` et les journaux de comparaison : retrait du banc et de ses artefacts après vérification.
