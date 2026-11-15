# Encodeur et décodeur GCR Commodore 1541/1571

- [x] Modèles et encodage
  - [x] Créer `src/MediaEngine/src/Codecs/Commodore/Commodore1541.h`, `Commodore1541Constants.h`, `Commodore1541Encoder.cpp` : contrat commun d’encodeur, identifiants, attributs de piste/disque, en-têtes, données et gaps fidèles au C#, en réutilisant les primitives GCR déjà portées.
- [x] Décodage
  - [x] Créer `src/MediaEngine/src/Codecs/Commodore/Commodore1541Decoder.cpp` : collecte des sync/en-têtes/données, enregistrements tronqués, appariement et intégrité nullable, structures ordonnées et descriptions localisables, confiance et annulation.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compilation, constantes et diagnostics structurés.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/commodore-gcr.json` et les 29 catalogues de langue, modifier `scripts/translations/ignores/common.txt` : erreurs/descriptions, génération Argos, relecture et correction.
  - [x] Modifier `src/Translations/Catalogs/Languages/ko-KR/MediaEngine/commodore900-gcr.json` : conserver l’invariant technique FormatData désormais explicitement protégé dans la liste commune.
- [x] Validation
  - [x] Créer `build/verification-commodore-gcr/Reference.csproj`, `Reference.cs`, `Contracts.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparer les sources C# originales pour l’encodage/flux et toutes les données de décodage, cas tronqués/corrompus/non appariés et attributs/limites, annulation et traductions en Debug/Release.
  - [x] Supprimer `build/verification-commodore-gcr/`, `build/commodore-gcr-reference.log`, `build/commodore-gcr-bench-debug.log`, `build/commodore-gcr-bench-release.log` après validation.
