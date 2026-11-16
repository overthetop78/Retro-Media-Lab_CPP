# Creative Voice séquentiel

- [x] Reader/writer VOC du profil C# Caprice32
  - [x] Créer `src/MediaEngine/src/Formats/Tape/Voc/Constants.h`, `Reader.h`, `Reader.cpp`, `Writer.cpp` : blocs son/continuation/silence/marqueur/texte, PCM signé 16 bits commun, conversion mono 8 bits et publication atomique avec continuations 24 bits.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, `src/MediaEngine/src/Images/ReadRejection.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/voc-tape.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/voc-tape.json` : erreurs traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj`, `.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-voc-tape/CMakeLists.txt`, `main.cpp` : blocs du profil, erreurs, conversion des profondeurs, limite des blocs 24 bits et annulation.
  - [x] Supprimer `build/verification-voc-tape/` et ses artefacts temporaires après consignation.

Validation et limites consignées dans docs/validation/media/voc-tape.md.
