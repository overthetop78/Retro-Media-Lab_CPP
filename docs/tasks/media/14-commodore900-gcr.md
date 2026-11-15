# Codec de piste Commodore 900 GCR

- [x] Modèles et encodage commun
  - [x] Créer `src/MediaEngine/src/Decoding/Models.h`, `LocalizedText.h`, `LocalizedText.cpp` : structures/secteurs/résultats possédés et descriptions résolues à la demande par clé et arguments imbriqués.
  - [x] Créer `src/MediaEngine/src/Encoding/Track.h`, `Constants.h`, `Track.cpp`, `Bits.h`, `Bits.cpp` : requêtes et secteurs, validations communes, production de révolution, primitives binaires/FM/MFM/gaps et attributs.
- [x] Codec Commodore 900
  - [x] Créer `src/MediaEngine/src/Codecs/Commodore/Commodore900.h`, `Commodore900Constants.h`, `Commodore900Encoder.cpp`, `Commodore900Decoder.cpp` : en-têtes/données GCR, synchronisations, appariement, intégrité, confiance et descriptions traduisibles.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` et `scripts/translations/ignores/common.txt` : compilation, constantes, diagnostics et invariant GCR ; régénérer les projets Visual C++.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/commodore900-gcr.json` et les 29 catalogues de langue : erreurs d’encodage et descriptions/états d’intégrité, génération Argos et relecture.
- [x] Validation
  - [x] Créer `build/verification-commodore900-gcr/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparer aux sources C# originales les bits/intervalles, secteurs/structures, checksums/positions, troncatures, corruption, appariement, aides communes et annulation en Debug et Release.
  - [x] Supprimer `build/verification-commodore900-gcr/` et `build/commodore900-gcr-reference.log`, `build/commodore900-gcr-bench-debug.log`, `build/commodore900-gcr-bench-result.log`, `build/commodore900-gcr-bench-release.log` après validation.
