# Conteneur et codec Atom TAP

- [x] Lecture, reconstruction et écriture Atom TAP
  - [x] Créer `src/MediaEngine/src/Formats/Tape/AtomTap/Constants.h`, `Reader.h`, `Reader.cpp`, `Reconstruction.cpp`, `Codec.h`, `Decode.cpp`, `Serial.cpp`, `Encode.cpp`, `Writer.cpp` : constantes C#, fichiers TAP successifs, reconstruction des blocs série, checksums et séquences, conservation des données non décodées, sérialisation des fichiers complets et écriture atomique.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/atom-tap.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/atom-tap.json` : rejets et diagnostics traduits.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj`, `.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-atom-tap/CMakeLists.txt`, `main.cpp` : fichiers successifs et vides, séquence multi-blocs, flags et checksums, série incomplète, conservation, rejets et annulation atomique.
  - [x] Supprimer `build/verification-atom-tap/` et ses artefacts temporaires après consignation.

- [x] Protéger les noms techniques
  - [x] Modifier `scripts/translations/ignores/common.txt` : conserver Atom et Atom TAP dans les traductions.
