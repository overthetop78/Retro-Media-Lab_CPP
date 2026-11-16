# Raccordement des writers SCP/HFE aux pistes protégées

- [x] Catalogue et adapters
  - [x] Modifier `src/MediaEngine/src/Formats/Definitions/DiskDefinitions.h`, `Reader.cpp` et `Constants.h` : exposer les identifiants dont toutes les pistes utilisent le codec bitcell, avec la même règle C#.
  - [x] Créer `src/MediaEngine/src/Formats/Scp/Adapter.h` et `Adapter.cpp`, modifier `Scp/Constants.h` : exposer le writer SCP avec les cibles bitcell et raw.scp, convertir les révolutions et écrire via le backend atomique.
  - [x] Modifier `src/MediaEngine/src/Formats/Hfe/Adapter.h`, `Adapter.cpp` et `Constants.h` : exposer le writer HFE pour les cibles bitcell/raw.hfe, conserver le prédicat de représentation exacte et les métadonnées du document.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler l'adapter SCP.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-flux-writer-adapters/CMakeLists.txt` et `main.cpp` : vérifier les cibles issues des ressources, les extensions et les sorties SCP/HFE depuis la représentation commune.
  - [x] Supprimer `build/verification-flux-writer-adapters/` et ses sorties/binaires après consignation.
