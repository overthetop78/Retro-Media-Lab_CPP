# Codecs de bande Spectrum et MSX

- [x] Décodeurs de blocs et PCM
  - [x] Modifier `src/MediaEngine/src/Formats/Tape/SpectrumTap/Constants.h` et `src/MediaEngine/src/Formats/Tape/MsxCas/Constants.h` : identifiants et paramètres techniques partagés des codecs.
  - [x] Créer `src/MediaEngine/src/Decoding/Sequential/Spectrum/Decoder.h`, `Decoder.cpp`, `Pulses.h`, `Pulses.cpp` : blocs TAP/TZX, XOR, détection des impulsions Spectrum en PCM et conservation des segments non décodés.
  - [x] Créer `src/MediaEngine/src/Decoding/Sequential/Msx/Decoder.h` et `Decoder.cpp` : contexte CAS/TSX/PCM, types de fichiers MSX, FSK 8N2 et propagation des messages de signal.
- [x] Encodeur Spectrum
  - [x] Créer `src/MediaEngine/src/Encoding/Sequential/Spectrum/Encoder.h` et `Encoder.cpp` : sorties TAP/TZX/PCM à partir des blocs validés et contrôle des allocations.
- [x] Intégration
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger l’invariant Spectrum et les identifiants techniques des codecs pour les générations suivantes.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/spectrum-msx-codecs.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/spectrum-msx-codecs.json` : erreurs et avertissements traduits.
  - [x] Modifier `src/Translations/Catalogs/Languages/cs-CZ/MediaEngine/sequential-pcm.json` et `src/Translations/Catalogs/Languages/th-TH/MediaEngine/spectrum-tzx.json` : corriger les deux invariants PCM/Spectrum signalés par l’audit commun.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : intégrer les codecs à la compilation et à la solution.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-spectrum-msx/CMakeLists.txt` et `main.cpp` : sélection des contextes, données et intégrités, sources conservées, aller-retour des codecs via les vrais conteneurs et PCM, profils, limites et annulation.
  - [x] Supprimer `build/verification-spectrum-msx/` et `build/spectrum-msx-smoke.log` après consignation.
