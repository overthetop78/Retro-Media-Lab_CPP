# Conversion des pistes protégées vers SCP

- [x] Modèle de sortie SCP
  - [x] Modifier `src/MediaEngine/src/Formats/Scp/ProtectedImage.h`, créer `OutputImage.cpp` : convertir les révolutions communes, conserver les métadonnées SCP, choisir la résolution et les faces et borner les conversions numériques selon le C#.
  - [x] Modifier `src/MediaEngine/src/Formats/Scp/Constants.h` : nommer clés, masque de drapeaux et limites utilisés par cette conversion.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler `Formats/Scp/OutputImage.cpp`.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-protected-scp-writing/CMakeLists.txt` et `main.cpp` : vérifier conversion de résolutions mélangées, métadonnées, nombre commun de révolutions et refus des pistes sans révolutions.
  - [x] Supprimer `build/verification-protected-scp-writing/` après consignation.
