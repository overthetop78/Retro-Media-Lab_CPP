# Writers ADF/ADFS, BBC DFS et CPCEMU

- [x] Raccordements
  - [x] Créer `src/MediaEngine/src/Formats/Adf/WriterAdapters.h` et `WriterAdapters.cpp`, modifier `Adf/Constants.h` : exposer les writers Acorn et Amiga avec les couples format/extension et prédicats C#.
  - [x] Créer `src/MediaEngine/src/Formats/BbcDfs/WriterAdapter.h` et `WriterAdapter.cpp`, modifier `BbcDfs/Constants.h` : exposer SSD/DSD selon le nombre de faces du profil.
  - [x] Créer `src/MediaEngine/src/Formats/CpcDsk/WriterAdapter.h` et `WriterAdapter.cpp`, modifier `CpcDsk/Constants.h` : exposer les cibles Amstrad et CPCEMU, construire le conteneur depuis les secteurs et choisir automatiquement Extended quand Standard ne peut pas préserver la disposition.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler les quatre adapters.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-adf-dfs-cpc-writers/CMakeLists.txt` et `main.cpp` : vérifier les couples profil/extension, les prédicats géométriques et une écriture DSK étendue conservant piste absente et états sectoriels.
  - [x] Supprimer `build/verification-adf-dfs-cpc-writers/` et sa sortie synthétique après consignation.
