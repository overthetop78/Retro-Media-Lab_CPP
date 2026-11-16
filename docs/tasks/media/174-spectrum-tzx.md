# Conteneurs Spectrum TAP et TZX

- [x] Structures et écritures
  - [x] Créer `src/MediaEngine/src/Formats/Tape/SpectrumTap/Constants.h`, `Reader.h`, `Reader.cpp`, `Writer.cpp` : framing TAP borné, XOR, métadonnées et écriture atomique des blocs valides.
  - [x] Créer `src/MediaEngine/src/Formats/Tape/Tzx/Constants.h`, `Reader.h`, `Reader.cpp`, `Blocks.h`, `Blocks.cpp`, `Writer.cpp` : constantes partagées, framing TZX/CDT/TSX, contrôle des sauts/boucles/appels et conservation des payloads.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/spectrum-tzx.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/spectrum-tzx.json` : diagnostics traduits.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj`, `.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-spectrum-tzx/CMakeLists.txt`, `main.cpp` : structure/XOR TAP, familles de longueurs TZX, flux de contrôle, versions, inconnus, aller-retour et annulation atomique.
  - [x] Supprimer `build/verification-spectrum-tzx/` et ses artefacts après consignation.
