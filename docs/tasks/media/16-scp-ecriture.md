# Écriture SCP et révolutions synthétiques

- [x] Encodage du conteneur
  - [x] Créer `src/MediaEngine/src/Formats/Scp/Writer.h`, `Writer.cpp`, `Synthetic.cpp` et modifier `Constants.h` : validation des pistes/révolutions, plan d’offsets et checksum, mots de flux avec report, sortie bornée et publication atomique ; conversion temporelle des pistes encodées et constantes de capture.
  - [x] Modifier `src/MediaEngine/src/Encoding/Constants.h` : constante partagée de durée d’un tick d’encodeur en nanosecondes.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` : compilation et diagnostics traduisibles du writer.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/scp-writer.json` et les 29 catalogues de langue : génération Argos puis relecture des erreurs.
- [x] Validation
  - [x] Créer `build/verification-scp-writer/Reference.csproj`, `Reference.cs`, `Contracts.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : comparaison aux sources C# originales des octets/offsets/checksum, reports et limites, conversion temporelle, annulation/erreurs/atomicité et trajet GCR → SCP → secteurs en Debug et Release.
  - [x] Supprimer `build/verification-scp-writer/`, `build/scp-writer-reference.log`, `build/scp-writer-bench-debug.log`, `build/scp-writer-bench-release.log` après validation.
