# Pistes HP MMFM

- [x] Transformations et pistes
  - [x] Créer `src/MediaEngine/src/Codecs/HpMmfm/Constants.h`, `Mmfm.h`, `Payload.cpp` : permutation par paires et inversion partagée des bits, helpers de lecture et contrats des pistes.
  - [x] Créer `src/MediaEngine/src/Codecs/HpMmfm/Encoder.cpp`, `Decoder.cpp` : identité inversée, CRC commun, recherche bornée des données et consommation unique des marques avec les états d’intégrité source.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compiler les sources et déclarer erreurs/constantes nommées.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/hp-mmfm.json` et ses 29 catalogues : erreurs de taille/plage/paire et descriptions CRC ; Argos puis relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger HP et MMFM.
- [x] Validation
  - [x] Créer `build/verification-hp-mmfm/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison C# des permutations, lectures directes, limites de recherche, CRC, réutilisation des données, cellules/révolutions/résultats de piste, erreurs, langues, registre et annulation Debug/Release.
  - [x] Supprimer `build/verification-hp-mmfm/` et ses journaux après vérification : retrait du banc et de ses artefacts temporaires.
