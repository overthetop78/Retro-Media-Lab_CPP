# Adapters d’écriture Apridisk / RX02

- [x] Raccordement aux writers communs
  - [x] Modifier `src/MediaEngine/src/Formats/Apridisk/Constants.h`, `Adapter.h` et `Adapter.cpp` : exposer le writer `apridisk`, sélectionner la cible DSK Apricot, valider la géométrie et réétiqueter la représentation avant écriture comme en C#.
  - [x] Modifier `src/MediaEngine/src/Formats/Rx02/Constants.h`, `Adapter.h` et `Adapter.cpp` : exposer le writer `dec-rx02`, sélectionner la cible IMG RX02 et conserver la validation stricte du profil natif.

Compilation incrémentale du raccordement ; aucune nouvelle batterie de codecs ni nouveau catalogue nécessaire.
