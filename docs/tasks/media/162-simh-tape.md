# Objets de bande SIMH

- [x] Conteneur SIMH séquentiel
  - [x] Créer `src/MediaEngine/src/Formats/Tape/Simh/Constants.h`, `Reader.h`, `Parser.h`, `Parser.cpp`, `Reader.cpp`, `Writer.cpp` : validation complète du framing, records, erreurs, marqueurs et copie atomique ordonnée.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, `src/MediaEngine/src/Images/ReadRejection.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/simh-tape.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/simh-tape.json` : rejets et avertissements traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `scripts/translations/ignores/common.txt`, `RetroMediaLab.vcxproj`, `.filters` : compilation, constantes et invariant SIMH.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-simh-tape/CMakeLists.txt`, `main.cpp` : mots encadrants, records pairs/impairs, marqueurs, drapeau erreur, tri stable, bornes et annulation atomique.
  - [x] Supprimer `build/verification-simh-tape/` avec ses données et binaires temporaires après consignation.

Validation et limites dans docs/validation/media/simh-tape.md.
