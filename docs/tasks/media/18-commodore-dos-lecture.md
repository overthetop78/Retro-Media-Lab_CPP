# Lecture et exploration Commodore DOS

- [x] Primitives et modèles
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/CommodoreDos.h`, `src/MediaFileSystems/src/Primitives/Petscii.h`, `Petscii.cpp`, `PetsciiConstants.h` : API de volume, codec PETSCII lecture/écriture fidèle aux unités UTF-16 et types/commentaires traduisibles.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Commodore/Dos/Constants.h`, `Internal.h`, `Layout.cpp`, `Sector.cpp` : dispositions 1541/1571/1581, conversion des coordonnées et états de lecture précis.
- [x] Lecture des volumes
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Commodore/Dos/Directory.cpp`, `File.cpp`, `Bam.cpp`, `Reader.cpp` : plausibilité, chaînes et contenus partiels, avertissements, noms/types/attributs REL, BAM des deux faces et espace libre indéterminé.
  - [x] Modifier `src/MediaEngine/src/Formats/CommodoreDisks/Images.h`, `Reader.cpp` : exploration par MediaFileSystems puis appel à l’analyse, sans accès direct de MediaEngine à MediaAnalysis.
  - [x] Modifier `src/MediaFileSystems/src/Exploration/EntryAnalysis.cpp` : transmettre le type invariant des commentaires Commodore structurés à MediaAnalysis, pour conserver la classification PRG sans extension indépendamment de la langue.
- [x] Construction et traductions
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, `include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` et `scripts/quality/audit_literals.py` : compilation, constantes nommées et diagnostics traduisibles ; régénérer les projets Visual C++.
  - [x] Modifier `scripts/translations/ignores/common.txt` et créer `src/Translations/Catalogs/Constants/MediaFileSystems/commodore-dos.json` ainsi que les 29 catalogues de langue : erreurs/avertissements, commentaires de types, génération Argos et relecture.
- [x] Validation
  - [x] Créer `build/verification-commodore-dos/Reference.csproj`, `Reference.cs`, `Contracts.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : compiler les algorithmes C# originaux et comparer PETSCII, reconnaissance, volumes/fichiers/BAM, contenus partiels et diagnostics, exploration/analyse et annulation en Debug et Release.
  - [x] Supprimer `build/verification-commodore-dos/`, `build/commodore-dos-reference.log`, `build/commodore-dos-bench-debug.log`, `build/commodore-dos-bench-release.log` après validation.
