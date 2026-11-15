# ST/MSA : transformation de géométrie et écriture

- [x] Transformation et writer ST
  - [x] Créer `src/MediaEngine/src/Formats/St/Transform.h`, `Transform.cpp`, `Writer.h` et `Writer.cpp` : préserver capacité, nombre et taille des secteurs, reconstruire les adresses physiques, conserver les métadonnées et ne compléter les secteurs absents que sur demande explicite SCP.
  - [x] Modifier `src/MediaEngine/src/Formats/St/Constants.h` : constantes d’adressage et de diagnostics géométriques.
- [x] Adapter commun
  - [x] Modifier `src/MediaEngine/src/Formats/Atari/Adapters.h`, `Adapters.cpp` et `Constants.h` : exposer la cible ST/MSA, transformer selon le profil demandé et compléter uniquement lorsque l’extension de source est SCP comme dans la composition C#.
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`) et créer les catalogues `Constants/MediaEngine/st-writing.json` et `Languages/<langue>/MediaEngine/st-writing.json` sous `src/Translations/Catalogs/` : erreurs de transformation et de writer traduites.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler transformation et writer.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-st-msa-writing/CMakeLists.txt` et `main.cpp` : vérifier conservation des secteurs/métadonnées, nouvelle adresse, refus de perte, traitement des absences SCP et sélection ST/MSA.
  - [x] Supprimer `build/verification-st-msa-writing/` après consignation.
