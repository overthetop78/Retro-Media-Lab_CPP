# Codec physique et lecteur logique Adaptec

- [x] Codec de pistes
  - [x] Modifier `src/MediaEngine/src/Formats/HardDisk/Adaptec/Constants.h` : compléter les constantes de piste, champs, paramètres et contrôle d'erreur C#.
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Adaptec/Codec.h`, `Internal.h`, `ErrorCheck.cpp`, `Encoding.cpp`, `Decoding.cpp`, `WriteWindow.cpp` : champs MFM circulaires, contrôles enregistrés, secteurs absents et fenêtre DATA sans modification de l'image source.
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Adaptec/Geometry.h` et `Geometry.cpp` : relire les paramètres physiques depuis les identités de piste, avec validation des adresses et drapeaux.
- [x] Lecture logique
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Adaptec/SectorReader.h` et `SectorReader.cpp` : géométrie découverte, un seul cache de piste, recherche des adresses déplacées, lecture exacte et adaptation aux sources de blocs avec possession explicite.
  - [x] Modifier les diagnostics MediaEngine ; créer `src/Translations/Catalogs/Constants/MediaEngine/adaptec.json` et `Languages/<langue>/MediaEngine/adaptec.json` : erreurs traduites du codec et du lecteur.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `.filters` : compiler et référencer les fichiers.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-adaptec/CMakeLists.txt` et `main.cpp` : contrôle d'erreur indépendant, champs valides/endommagés/absents, géométrie, fenêtre d'écriture, cache et accès logique aux secteurs.
  - [x] Supprimer `build/verification-adaptec/` avec son programme et ses binaires après consignation.
