# RIFF/WAVE séquentiel

- [x] Lecture/écriture WAVE sans conversion PCM
  - [x] Créer `src/MediaEngine/src/Formats/Tape/Wav/Constants.h`, `Reader.h`, `Reader.cpp`, `Writer.cpp` : profil PCM borné C#, chunks RIFF, segments entrelacés possédés, métadonnées, avertissement de fin et writer atomique sans perte.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, `src/MediaEngine/src/Images/ReadRejection.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/wave-tape.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/wave-tape.json` : erreurs et avertissement traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj`, `.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-wave-tape/CMakeLists.txt`, `main.cpp` : profils, chunks multiples/inconnus, précision native, bornes RIFF, publication atomique et annulation.
  - [x] Supprimer `build/verification-wave-tape/` et ses artefacts temporaires après consignation.

Validation et limites consignées dans docs/validation/media/wave-tape.md.
