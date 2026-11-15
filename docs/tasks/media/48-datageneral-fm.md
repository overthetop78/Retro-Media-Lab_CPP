# Codec Data General 2F

- [x] Pistes Data General
  - [x] Créer `src/MediaEngine/src/Codecs/DataGeneral/Constants.h`, `Fm.h`, `Checksum.cpp`, `Encoder.cpp`, `Decoder.cpp` : checksum avec zéro terminal, adresses composées, encodage FM et décodage par paires de synchronisations adjacentes.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compiler les sources, diagnostics et constantes nommés.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/datageneral-fm.json` et ses 29 catalogues : taille/plage, descriptions d’identité et de checksum ; Argos puis relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger Data General et 2F.
- [x] Validation
  - [x] Créer `build/verification-datageneral/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparer les sources originales pour checksum, adresses, secteurs multiples, synchronisations adjacentes/internes, limites 32/256 cellules, troncatures, erreurs et traductions, annulation et registre Debug/Release.
  - [x] Supprimer `build/verification-datageneral/` et les journaux associés après vérification : retrait du banc temporaire et de ses artefacts.
