# Signal de bande et conversion UEF

- [x] Signal possédé et transformations
  - [x] Créer `src/MediaEngine/src/Models/Sequential/TapeSignal.h`, `TapeSignal.cpp` : signal mono 16 bits et blocs possédés, repères facultatifs et intégrité à trois états.
  - [x] Créer `src/MediaEngine/src/Formats/Tape/Common/Signal.h`, `SignalConstants.h`, `PcmSignal.cpp`, `UefSignal.cpp`, `UefDocument.cpp` : extraction du premier canal, synthèse UEF/Atom et repères, transformation des passages par zéro en chunks UEF sans interprétation machine.
  - [x] Modifier `src/MediaEngine/src/Formats/Tape/Common/Pcm.h`, `PcmDocument.cpp` : adaptation du modèle de signal sans dupliquer l’encodage PCM.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/tape-signal.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/tape-signal.json` : diagnostics traduits.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj`, `.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-tape-signal/CMakeLists.txt`, `main.cpp` : propriété, PCM natif, trames/parité/phase, cycles/gaps, repères, transformation de signaux arbitraires et limites/annulation.
  - [x] Supprimer `build/verification-tape-signal/` et ses artefacts après consignation.
