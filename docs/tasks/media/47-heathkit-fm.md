# Codec Heathkit FM

- [x] Primitives et pistes
  - [x] Créer `src/MediaEngine/src/Primitives/BitConstants.h`, `Bits.h` et modifier `src/MediaEngine/src/Codecs/Emu/Fm.h` : partager l’inversion des bits d’un octet sans dépendance entre formats.
  - [x] Créer `src/MediaEngine/src/Codecs/Heathkit/Constants.h`, `Fm.h`, `Record.cpp`, `Encoder.cpp`, `Decoder.cpp` : records inversés/checksum rotatif commun, volume de piste, lecture/appairage et intégrité des secteurs.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compiler les sources et déclarer erreurs/constantes.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/heathkit-fm.json` et ses 29 catalogues : erreurs de taille/plage/record vide, descriptions d’en-têtes et données ; Argos puis relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger Heathkit.
- [x] Validation
  - [x] Créer `build/verification-heathkit/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison C# des records, bits, volumes, pistes, appairages et limites, intégrité, troncatures, langues et annulation Debug/Release ; contrôler aussi l’inversion partagée d’E-mu.
  - [x] Supprimer `build/verification-heathkit/` et ses journaux après vérification : retrait du banc et de ses artefacts temporaires.
