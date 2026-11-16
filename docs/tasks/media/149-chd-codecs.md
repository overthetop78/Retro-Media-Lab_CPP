# Codecs communs CHD

- [x] Bits et Huffman
  - [x] Créer `src/MediaEngine/src/Formats/Common/Chd/CodecConstants.h`, `Bitstream.h`, `Huffman.h`, `Huffman.cpp` : lectures MSB, arbres RLE/Huffman canoniques et décodage exact depuis le C#.
- [x] Décompression et audio
  - [x] Créer `src/MediaEngine/src/Formats/Common/Chd/Compression.h`, `Compression.cpp` : DEFLATE brut, LZMA et Zstandard avec les bibliothèques déjà embarquées ; mêmes paramètres CHD, limites et consommations vérifiées, libération systématique et annulation.
  - [x] Créer `src/MediaEngine/src/Formats/Common/Chd/Flac.h`, `Flac.cpp` : trames sans STREAMINFO, CRC, sous-trames constantes/brutes/prédites, résidus et canaux stéréo depuis le C#.
  - [x] Modifier les diagnostics MediaEngine, créer `src/Translations/Catalogs/Constants/MediaEngine/chd-codecs.json` et `Languages/<langue>/MediaEngine/chd-codecs.json` : erreurs traduites et invariants communs.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : lier les codecs embarqués et référencer les nouvelles sources et constantes.
- [x] Validation ciblée
  - [x] Créer `build/verification-chd-codecs/CMakeLists.txt` et `main.cpp` : Huffman et trames synthétiques, consommation des entrées, DEFLATE/LZMA/Zstandard, FLAC stéréo/CRC et rejets ; annulation.
  - [x] Supprimer `build/verification-chd-codecs/` et ses données et binaires temporaires après consignation.

Bloc terminé : compilation MediaEngine, contrôle ciblé, audit et compilation des 30 catalogues réussis. Le conteneur CHD et son intégration restent distincts.
