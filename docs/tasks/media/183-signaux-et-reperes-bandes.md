# Accès aux signaux et repères de bandes

- [x] Lecture et écriture communes
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/TapeFormatIds.h` : identifiants stables des treize formats séquentiels du C#.
  - [x] Créer `src/MediaEngine/src/Formats/Tape/Common/SignalIoConstants.h`, `SignalIO.cpp`, `SignalBlocks.h`, `SignalBlocks.cpp`, `BlockIndex.cpp` ; modifier `Signal.h` : accès aux cinq conteneurs du C#, écriture WAV/UEF/CSW/RAW, association des blocs au signal, porteuses et index d'affichage sans réencodage.
  - [x] Modifier `src/MediaEngine/src/Models/Sequential/TapeSignal.h` et `TapeSignal.cpp` : ajouter les blocs et leurs repères sans recopier les échantillons possédés.
- [x] Intégration
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/tape-signal-access.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/tape-signal-access.json` : erreur de format traduisible.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compilation, constantes et solution.
- [x] Vérification ciblée de l'assemblage
  - [x] Créer `build/verification-tape-signal-access/CMakeLists.txt` et `main.cpp` : lecture/écriture de signal, association de blocs et repères, index sans copie ni perte des canaux, refus et annulation.
  - [x] Supprimer `build/verification-tape-signal-access/` et ses sorties temporaires après consignation.
