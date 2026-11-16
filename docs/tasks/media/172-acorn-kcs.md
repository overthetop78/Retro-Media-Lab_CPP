# Protocole de bande Acorn KCS

- [x] Blocs, décodage et encodage
  - [x] Créer `src/MediaEngine/src/Formats/Tape/Acorn/Constants.h`, `Blocks.h`, `Blocks.cpp` : champs CFS, CRC partagé, validation des métadonnées et reconstruction des blocs.
  - [x] Créer `src/MediaEngine/src/Decoding/Sequential/Acorn/AcornDecoder.h`, `AcornDecoder.cpp` : blocs implicites UEF, PCM continu, sélection 1200/300 bauds validée par CRC et conservation des segments.
  - [x] Créer `src/MediaEngine/src/Encoding/Sequential/Acorn/AcornEncoder.h`, `AcornEncoder.cpp`, `AcornOutput.h`, `AcornOutput.cpp` : sélection, reconstruction UEF et PCM FSK avec porteuses/gaps et borne mémoire.
  - [x] Modifier `src/MediaEngine/src/Decoding/Sequential/Models.h`, `Models.cpp` : renumérotation des blocs conservant les données immuables partagées.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/acorn-kcs.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/acorn-kcs.json` : diagnostics traduits.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj`, `.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-acorn-kcs/CMakeLists.txt`, `main.cpp` : CRC/champs, UEF implicite/cycles, PCM et frontières de segments, débits 1200/300, sélections/rejets et annulation.
  - [x] Supprimer `build/verification-acorn-kcs/` et ses artefacts temporaires après consignation.
