# TeleDisk : décompression avancée

- [x] Flux LZSS / Huffman adaptatif
  - [x] Créer `src/MediaEngine/src/Formats/TeleDisk/Advanced.h` et `Advanced.cpp` : porter lecture des bits, dictionnaire glissant, arbre adaptatif, tables de positions et reconstruction des fréquences ; assurer annulation et nettoyage RAII.
  - [x] Modifier `src/MediaEngine/src/Formats/TeleDisk/Constants.h` : constantes de fenêtre, arbre, limites et tables reprises du C#.
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`) et créer `src/Translations/Catalogs/Constants/MediaEngine/teledisk-advanced.json` ainsi que `Languages/<langue>/MediaEngine/teledisk-advanced.json` : traduire le dépassement de la limite décompressée.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler le décompresseur.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-teledisk-advanced/CMakeLists.txt`, `main.cpp` et `fixtures.py` : produire des flux synthétiques couvrant littéraux, référence au dictionnaire et reconstruction de l’arbre après saturation des fréquences et limite de taille décompressée ; identifier précisément toute différence nécessaire avec le C#.
  - [x] Supprimer `build/verification-teledisk-advanced/` et ses sorties après consignation.
