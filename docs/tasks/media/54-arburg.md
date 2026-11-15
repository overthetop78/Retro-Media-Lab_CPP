# Blocs Arburg système/données

- [x] Primitives et pistes
  - [x] Créer `src/MediaEngine/src/Codecs/Arburg/Constants.h`, `Blocks.h`, `Checksum.cpp` : définitions des deux blocs, checksum additif faible/fort, remplissage, reconstruction des blocs complets et validations de bornes.
  - [x] Créer `src/MediaEngine/src/Codecs/Arburg/System.cpp` : codage variable LSB-first 01/001, lecture et position finale avec contrôle des bornes.
  - [x] Créer `src/MediaEngine/src/Codecs/Arburg/Encoder.cpp`, `Decoder.cpp` : sélection par attribut sectoriel, double FM inversé, deux balayages indépendants, ordre des secteurs/octet et tri stable des structures.
- [x] Construction et langues
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : sources compilées, diagnostics et constantes nommées.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger Arburg.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/arburg.json` et ses 29 catalogues : erreurs de tailles/bornes et descriptions des deux variantes ; Argos puis relecture.
- [x] Validation
  - [x] Créer `build/verification-arburg/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparer les sources C# originales, checksum/blocs, transformation système et positions, pistes FM/système/mélangées, troncatures, checksum incorrect, bornes, langues, annulation et registre Debug/Release.
  - [x] Supprimer `build/verification-arburg/` et ses journaux après vérification : retrait du banc et de ses artefacts temporaires.
