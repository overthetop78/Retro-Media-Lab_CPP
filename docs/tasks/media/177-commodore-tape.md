# Bandes Commodore TAP et PCM

- [x] Conteneur et protocole partagés
  - [x] Créer `src/MediaEngine/src/Formats/Tape/CommodoreTap/Constants.h`, `Profile.h`, `Profile.cpp` : signatures, versions, horloges, métadonnées et validation du profil TAP.
  - [x] Créer `src/MediaEngine/src/Formats/Tape/CommodoreTap/Reader.h`, `Reader.cpp`, `Writer.cpp` : pulses TAP v0/v1/v2, plages sources, temps et écriture atomique des durées représentables.
  - [x] Créer `src/MediaEngine/src/Formats/Tape/CommodoreTap/Protocol.h`, `Protocol.cpp` : trames ROM Commodore, paires d’impulsions, parité impaire, conservation des positions sources et génération des cycles.
- [x] Codecs séquentiels
  - [x] Créer `src/MediaEngine/src/Decoding/Sequential/Commodore/Decoder.h`, `Decoder.cpp` : décodage des pulses TAP/PCM, intégrité de parité et segments non décodés.
  - [x] Créer `src/MediaEngine/src/Encoding/Sequential/Commodore/Encoder.h`, `Encoder.cpp` : encodage TAP/PCM des blocs validés, paramètres et allocations contrôlés.
- [x] Intégration
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/commodore-tape.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/commodore-tape.json` : diagnostics traduits.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `scripts/translations/ignores/common.txt`, `RetroMediaLab.vcxproj`, `RetroMediaLab.vcxproj.filters` : compilation, constantes et invariants.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-commodore-tape/CMakeLists.txt` et `main.cpp` : profils/horloges, pulses courts/étendus, trames/parité, conservation, aller-retour conteneurs et PCM, refus et annulation atomique.
  - [x] Supprimer `build/verification-commodore-tape/` et `build/commodore-tape-smoke.log` après consignation.
