# Bandes Amstrad CPC et intervalles PCM communs

- [x] Protocole CPC
  - [x] Créer `src/MediaEngine/src/Formats/Tape/Amstrad/Constants.h`, `Records.h`, `Records.cpp` : constantes, en-têtes Latin1, segments de 256 octets, CRC et reconstruction des records CPC.
  - [x] Créer `src/MediaEngine/src/Decoding/Sequential/Amstrad/Decoder.h`, `Decoder.cpp`, `Pulses.h`, `Pulses.cpp` : lecture CDT/TZX et PCM, association en-tête/données, intégrité et conservation des sources.
  - [x] Créer `src/MediaEngine/src/Encoding/Sequential/Amstrad/Encoder.h`, `Encoder.cpp` : records reconstruits, turbo CDT/TZX et PCM WAV/VOC, paramètres et limites.
- [x] Analyse des impulsions partagée
  - [x] Créer `src/MediaEngine/src/Formats/Tape/Common/PulseIntervals.h` et `PulseIntervals.cpp` : mesure des intervalles observés, silences, fin de signal et annulation.
  - [x] Modifier `src/MediaEngine/src/Formats/Tape/Common/PcmConstants.h` et `src/MediaEngine/src/Formats/Tape/SpectrumTap/Constants.h` : centraliser le seuil de silence commun.
  - [x] Modifier `src/MediaEngine/src/Formats/Tape/Tzx/Metadata.h`, `Metadata.cpp` et `src/MediaEngine/src/Decoding/Sequential/Spectrum/Decoder.cpp` : partager l’extraction bornée des plages de données standard/turbo/pures entre Spectrum et CPC.
  - [x] Modifier `src/MediaEngine/src/Decoding/Sequential/Spectrum/Pulses.cpp` et `src/MediaEngine/src/Decoding/Sequential/Commodore/Decoder.cpp` : utiliser cette mesure commune en conservant les reconnaissances propres aux protocoles.
- [x] Intégration
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/amstrad-cpc-tape.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/amstrad-cpc-tape.json` : diagnostics traduits.
  - [x] Modifier `src/Translations/Catalogs/Languages/{ru-RU,zh-Hans,zh-Hant,nl-NL,uk-UA}/MediaEngine/amstrad-fds.json` : rétablir les invariants CPC et corriger les anciens messages signalés par l’audit commun.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `scripts/translations/ignores/common.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compilation, constantes et invariants.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-amstrad-cpc-tape/CMakeLists.txt` et `main.cpp` : records/CRC, associations, défauts et données conservées, aller-retour CDT/WAV/VOC, limites et annulation ; régression PCM Spectrum/Commodore sur la mesure commune.
  - [x] Supprimer `build/verification-amstrad-cpc-tape/` et `build/amstrad-cpc-tape-smoke.log` après consignation.
