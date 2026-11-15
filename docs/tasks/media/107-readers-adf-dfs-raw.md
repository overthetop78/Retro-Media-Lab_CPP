# Contrats communs ADF, BBC DFS, IBM et MSX

- [x] Readers
  - [x] Créer `src/MediaEngine/src/Formats/Adf/Adapter.h` et `Adapter.cpp` : capacités, sonde native, lecture sectorielle et message structuré signalant une image partielle.
  - [x] Créer `src/MediaEngine/src/Formats/BbcDfs/Adapter.h` et `Adapter.cpp` : capacités SSD/DSD/ADL et alias bruts, sondes de catalogues puis construction du document commun.
  - [x] Créer `src/MediaEngine/src/Formats/Raw/Adapters.h` et `Adapters.cpp`, modifier `Constants.h` : readers IBM/MSX, préfixes, sondes et documents sectoriels en réutilisant les géométries et lecteurs natifs.
  - [x] Modifier `src/MediaEngine/src/Formats/Raw/Reader.cpp` : conserver l’état logique initial valide des secteurs complets IBM/MSX, comme les constructeurs C#, sans prétendre vérifier un CRC physique.
- [x] Intégration
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler les quatre readers, puis régénérer les projets Visual C++.
- [x] Vérification ciblée
  - [x] Créer `build/verification-adf-dfs-raw/CMakeLists.txt` et `main.cpp` : vérifier le raccordement des quatre readers, leurs sondes et les adresses sectorielles conservées.
  - [x] Supprimer `build/verification-adf-dfs-raw/` après consignation du résultat.
