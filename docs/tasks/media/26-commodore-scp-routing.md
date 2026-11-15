# Routage et entrée fichier des reconstructions Commodore SCP

- [x] Point d’entrée commun
  - [x] Modifier `src/MediaEngine/src/Formats/CommodoreDisks/FluxReader.h` et créer `Router.cpp` : surcharge capture et surcharge fichier, sélection exacte 1581/900 puis repli GCR conforme au C#, transmission progression/annulation sans nouveau parseur.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compilation du routeur.
- [x] Validation
  - [x] Créer `build/verification-commodore-scp-routing/Reference.csproj`, `Contracts.cs`, `Reference.cs`, `main.cpp`, `CMakeLists.txt` : dispatch du lecteur C# original, formats exacts/absents/inconnus/casse, cohérence capture/fichier/direct pour les trois familles, progression, annulation et exceptions.
  - [x] Supprimer `build/verification-commodore-scp-routing/`, `build/commodore-scp-routing-reference.log`, `build/commodore-scp-routing-bench-debug.log`, `build/commodore-scp-routing-bench-release.log` : retrait du banc et de ses captures après validation.
