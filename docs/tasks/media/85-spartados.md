# Système de fichiers Atari SpartaDOS

- [x] Lecture et structures SpartaDOS
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/SpartaDos.h`, `src/MediaFileSystems/src/FileSystems/Atari/SpartaDos/Constants.h`, `Internal.h`, `Primitives.cpp` : contrats sectoriels existants, champs/noms ASCII, flags, longueurs 24 bits et dates.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Atari/SpartaDos/Disk.cpp`, `Allocation.cpp`, `File.cpp`, `Directory.cpp`, `Reader.cpp` : en-têtes 1.x/2.x, cartes liées et backlinks, contenus creux, répertoires et cycles, occupation/attributs/métadonnées fidèles.
  - [x] Modifier CMake MFS, diagnostics et audit des constantes ; créer les catalogues SpartaDOS : intégration partagée assurée par le parent, aucun writer absent du C#.
- [x] Validation ciblée
  - [x] Créer `build/verification-spartados/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer les 12 sources C# originales sur en-tête, secteurs/maps, sparse, dates/noms, flags et répertoires ; annulation sans rejouer AtariDOS.
  - [x] Supprimer `build/verification-spartados/` et ses artefacts après validation.

- [x] Traductions SpartaDOS
  - [x] Modifier `src/Translations/Catalogs/Languages/*/MediaFileSystems/sparta-dos.json` : relire les 29 catalogues, préserver paramètres/SpartaDOS et corriger volume, boucles de cartes/répertoires, prédécesseur et octets.
