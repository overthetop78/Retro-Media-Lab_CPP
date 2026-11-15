# Adapters d’écriture sectorielle : Atari ATR, IBM, MSX, COHERENT et Atom

- [x] Raccordements aux writers communs
  - [x] Modifier `src/MediaEngine/src/Formats/Atari/Adapters.h`, `Adapters.cpp` et `Constants.h` : exposer le writer ATR pour ses trois profils Atari 8-bit et l’extension ATR, en conservant la validation native.
  - [x] Créer `src/MediaEngine/src/Formats/Raw/WriterAdapters.h` et `WriterAdapters.cpp`, modifier `Raw/Constants.h` : exposer les writers IBM IMA/IMG et MSX DSK, avec profils explicites et prédicats natifs.
  - [x] Créer `src/MediaEngine/src/Formats/Commodore900/WriterAdapter.h` et `WriterAdapter.cpp`, modifier `Commodore900/Constants.h` : exposer le writer COHERENT BIN/IMG sans transformation implicite du format.
  - [x] Créer `src/MediaEngine/src/Formats/AcornAtom/WriterAdapter.h` et `WriterAdapter.cpp`, modifier `AcornAtom/Constants.h` : exposer la cible Atom DSK, valider la géométrie indépendamment du nom source et écrire dans la géométrie cible.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler les adapters.
- [x] Contrôle du raccordement
  - [x] Créer `build/verification-sector-writer-adapters/CMakeLists.txt` et `main.cpp` : vérifier les associations profil/extension et le refus des géométries incompatibles avec les cinq writers ; aucune répétition des tests de codecs.
  - [x] Supprimer `build/verification-sector-writer-adapters/` après consignation.
