# Système de fichiers UCSD p-System

- [x] Structures et lecture UCSD
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Ucsd.h`, `src/MediaFileSystems/src/FileSystems/Ucsd/Constants.h`, `Internal.h`, `Primitives.cpp` : ordre des octets, types, noms Pascal ASCII et dates compactes avec contrats existants.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Ucsd/Header.cpp`, `Directory.cpp`, `Content.cpp`, `Reader.cpp` : en-tête court/long, lecture positionnelle, entrées traversant des blocs, plages/chevauchements, contenus et espace libre fidèles au C#.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FormatIds.h`, CMake MFS, diagnostics et audit des constantes ; créer les catalogues UCSD : intégrer `UcsdIbmMfm=ucsd.ibm.mfm` exact et les sources/diagnostics proposés, intégration partagée assurée par le parent.
- [x] Validation ciblée
  - [x] Créer `build/verification-ucsd/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : compiler les véritables sources C# UCSD et comparer dates/noms, deux ordres des octets, blocs absents, frontières d’entrées, tailles finales, plages et chevauchements, annulation.
  - [x] Supprimer `build/verification-ucsd/` et ses artefacts après validation.

- [x] Traductions UCSD
  - [x] Modifier `src/Translations/Catalogs/Languages/*/MediaFileSystems/ucsd.json` : relire les 29 catalogues avec répartition parent/agent, corriger textes/quantités/intervalle exclusif et préserver UCSD p-System et paramètres.
