# Lecture SCP et reconstruction Commodore 900

- [x] Conteneur et fenêtres
  - [x] Créer `src/MediaEngine/src/Formats/Scp/Constants.h`, `Scp.h`, `Models.cpp`, `Reader.cpp`, `Windows.cpp` : modèles possédés, en-tête/signatures/plages/checksum, lecture bornée des mots de flux et fenêtres entre révolutions ; lecture depuis octets ou source, sans cache implicite.
- [x] Reconstruction sectorielle
  - [x] Créer `src/MediaEngine/src/Decoding/Candidates.h` et `src/MediaEngine/src/Formats/Commodore900/FluxReader.h`, `FluxReader.cpp` : priorité d’intégrité stable, décodage des fenêtres, validation d’adresse/taille et image sectorielle creuse avec progression/annulation.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : construction, diagnostics et constantes nommées, couverture des clés MediaEngine dans l’audit ; régénérer les projets Visual C++.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/scp-reader.json` et les 29 catalogues correspondants : erreurs du conteneur/fenêtres/reconstruction, génération Argos et relecture.
- [x] Validation
  - [x] Créer `build/verification-scp-reader/Reference.csproj`, `Reference.cs`, `Contracts.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : compiler les algorithmes C# originaux avec contrats de banc minimaux, comparer en-têtes/pistes/intervalles/checksum/fenêtres/reconstruction, malformed/overflow/annulation/sources bornées et textes traduits en Debug et Release.
  - [x] Supprimer `build/verification-scp-reader/` et `build/scp-reader-reference.log`, `build/scp-reader-bench-debug.log`, `build/scp-reader-bench-release.log` après validation.
