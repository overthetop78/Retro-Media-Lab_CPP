# Reconnaissance et lecture Apple brute

- [x] Structures et sondes
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/AppleRecognition.h`, `AppleRecognitionConstants.h`, `src/MediaFileSystems/src/FileSystems/Apple/Recognition.cpp` : validation DOS/ProDOS, signatures Macintosh, en-têtes Lisa, marqueur SOS ; conserver les structures de fichiers dans MFS.
  - [x] Créer `src/MediaEngine/src/Formats/Apple/Probe.h`, `Probe.cpp`, `Raw.h`, `RawReader.cpp`, `SectorOrder.h`, `SectorOrder.cpp`, `Constants.h` : sonder les images, réordonner les secteurs DOS/ProDOS et construire les images Apple II et Macintosh avec la preuve de sélection du C#.
  - [x] Modifier `src/MediaEngine/src/Formats/FormatIds.h`, les `CMakeLists.txt` de MediaEngine/MediaFileSystems, `scripts/quality/audit_literals.py`, les diagnostics MediaEngine ; créer `src/Translations/Catalogs/Constants/MediaEngine/apple-raw.json` et les catalogues `Languages/` : compiler les nouveaux services et traduire leurs erreurs.
- [x] Vérification ciblée
  - [x] Créer `build/verification-apple-raw/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer les sondes, conversions, décisions de lecture, adresses et données avec les sources C# originales, y compris les priorités de reconnaissance et les entrées tronquées ; compiler les cibles affectées.
  - [x] Supprimer `build/verification-apple-raw/` et les journaux temporaires après validation.
