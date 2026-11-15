# BBC DFS — système de fichiers et conteneurs SSD/DSD

- [x] Contrats et primitives
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FormatIds.h` : identifiants partagés DFS/Atom ; créer `BbcDfs.h` dans ce même dossier : champs compactés, noms et API lecture.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Acorn/BbcDfs/Constants.h`, `Internal.h`, `Codec.cpp`, `Reader.cpp`, `Sides.cpp` : catalogues, champs 18/10 bits, fichiers incomplets, projections de faces empruntées sans copie des octets et diagnostics contextualisés.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h` et `src/Diagnostics.cpp` : diagnostics DFS et contexte de face séparé des textes traduits.
- [x] Conteneurs du moteur
  - [x] Modifier `src/MediaEngine/src/Models/Sectors/SectorImages.h`, `.cpp` et `SectorConstants.h` : numérotation sectorielle source paramétrable et identifiant enregistré de face, sans dupliquer le chargeur paresseux.
  - [x] Créer `src/MediaEngine/src/Formats/BbcDfs/BbcDfsImages.h`, `Constants.h`, `Reader.cpp`, `Writer.cpp` : quatre profils, règles SSD/DSD/IMG/IMA/ADL, secteurs tronqués, lecture/analyse et écriture atomique via la primitive linéaire partagée.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h` et `src/Diagnostics.cpp` : erreurs de lecture et cible DFS.
- [x] Construction et traductions
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `src/MediaFileSystems/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `scripts/translations/ignores/common.txt` ; régénérer les projets Visual C++ : sources DFS et constantes.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/bbc-dfs.json`, `Constants/MediaEngine/bbc-dfs.json` et les équivalents des 29 langues : Argos, relecture, clés/invariants/paramètres contrôlés.
- [x] Validation
  - [x] Créer `build/verification-dfs/` : banc C#/C++ pour lecteurs, champs/noms, catalogues connus et structurels, deux faces, images tronquées, écritures et réouverture/analyse, annulation et intégrité.
  - [x] Supprimer `build/verification-dfs/` et ses artefacts après validation.
