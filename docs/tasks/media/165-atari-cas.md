# Atari CAS et protocole FSK Atari

- [x] Conteneur CAS possédé et sans perte de chunks
  - [x] Créer `src/MediaEngine/src/Formats/Tape/AtariCas/Constants.h`, `Reader.h`, `Reader.cpp`, `Writer.cpp` : FUJI/baud/data/fsk, temps, noms et métadonnées, chunks inconnus conservés et writer atomique.
- [x] Fonctions FSK communes et décodeur Atari
  - [x] Créer `src/MediaEngine/src/Decoding/Sequential/Fsk/Constants.h`, `Serial.h`, `Result.cpp`, `Decode.cpp`, `Encode.cpp`, `src/MediaEngine/src/Decoding/Sequential/Compatibility.h`, `.cpp` : résultat possédé, classification des tons, récupération d’horloge, trames UART, génération PCM et compatibilité.
  - [x] Créer `src/MediaEngine/src/Decoding/Sequential/Atari/Decoder.h`, `Decoder.cpp`, `Records.cpp` : records standards et checksum, regroupement logique, FSK CAS et PCM, segments non décodés et diagnostics.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, `src/MediaEngine/src/Images/ReadRejection.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/atari-tape.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/atari-tape.json` : diagnostics traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `scripts/translations/ignores/common.txt`, `RetroMediaLab.vcxproj`, `.filters` : compilation, constantes et invariants CAS/FSK/FUJI.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-atari-tape/CMakeLists.txt`, `main.cpp` : conservation CAS, temps, UART/PCM et horloge, records/checksum, intégration du décodeur avec MFS, bornes et annulation.
  - [x] Supprimer `build/verification-atari-tape/` et ses artefacts temporaires après consignation.

Validation et limites dans docs/validation/media/atari-tape.md.
