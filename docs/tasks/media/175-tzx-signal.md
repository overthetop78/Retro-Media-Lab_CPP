# Signal TZX et conservation des segments

- [x] Décodeur et encodeur de signal
  - [x] Modifier `src/MediaEngine/src/Formats/Tape/Tzx/Constants.h` : constantes des champs, rôles temporels et paramètres partagés du signal.
  - [x] Créer `src/MediaEngine/src/Decoding/Sequential/Tzx/SignalDecoder.h`, `SignalDecoder.cpp`, `Expansion.h`, `Expansion.cpp` : résultat temporel, expansion standard/turbo/tonalité/séquence/données/enregistrement direct/pause et conservation des sources avec diagnostics.
  - [x] Créer `src/MediaEngine/src/Encoding/Sequential/Tzx/SignalEncoder.h`, `SignalEncoder.cpp` : encodage des blocs validés, conservation TZX et conversion des impulsions/pauses, avec ordre stable.
  - [x] Créer `src/MediaEngine/src/Formats/Tape/Tzx/Metadata.h` et `Metadata.cpp` : partager la lecture et le formatage des identifiants TZX entre décodeur et encodeur sans dépendance de l’encodage vers le décodage.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/tzx-signal.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/tzx-signal.json` : erreurs et avertissements traduits.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : intégrer le signal TZX dans la compilation et la solution.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-tzx-signal/CMakeLists.txt` et `main.cpp` : expansion temporelle, plages partagées, bits partiels, paramètres invalides, conservation, ordre, aller-retour avec les conteneurs et annulation.
  - [x] Supprimer `build/verification-tzx-signal/` et ses artefacts après consignation.
