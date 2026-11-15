# Atari DOS — systèmes de fichiers

- [x] Lecteur et composants
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/AtariDos.h`, `AtariDosConstants.h`, `src/MediaFileSystems/src/FileSystems/Atari/Dos/Constants.h`, `Internal.h` et `Primitives.cpp`, `File.cpp`, `Allocation.cpp`, `Directory.cpp`, `Reader.cpp` regroupés par responsabilité : porter les 14 fichiers C# de variantes Atari DOS, répertoire, chaînes/VTOC, tailles de secteurs et métadonnées avec les modèles MFS existants.
  - [x] Modifier les fichiers partagés de compilation, diagnostics, audit et identifiants et créer les catalogues `Constants/MediaFileSystems/atari-dos.json`, `Languages/*/MediaFileSystems/atari-dos.json` : intégrer les sources et diagnostics structurés traduits (agent parent).
  - [x] Modifier les catalogues `src/Translations/Catalogs/Languages/*/MediaFileSystems/atari-dos.json` : relire/corriger les traductions Argos et vérifier paramètres/invariants selon partage coordonné avec le parent.
- [x] Vérification ciblée
  - [x] Créer `build/verification-atari-dos/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer les sources C# originales sur variantes, secteurs 128/256 octets, répertoires, allocations/chaînes, métadonnées et erreurs ; vérifier annulation C++ sans rejouer les blocs précédents.
  - [x] Supprimer `build/verification-atari-dos/` et les artefacts après validation et intégration de l’inventaire par le parent.
