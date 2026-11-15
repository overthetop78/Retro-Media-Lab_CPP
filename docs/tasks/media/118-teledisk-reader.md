# TeleDisk : lecteur détaillé et reconnaissance

- [x] Conteneur et image logique
  - [x] Créer `src/MediaEngine/src/Formats/TeleDisk/Reader.h`, `Reader.cpp` et `Internal.h` : analyser les en-têtes, commentaires, CRC et secteurs, intégrer la décompression avancée et conserver les enregistrements détaillés.
  - [x] Créer `src/MediaEngine/src/Formats/TeleDisk/SectorImage.cpp` : classer IBM/UCSD à partir du boot et de la géométrie, conserver adresses enregistrées, ordre, erreurs et marques supprimées, reconstruire les emplacements manquants.
  - [x] Créer `src/MediaEngine/src/Formats/TeleDisk/Adapter.cpp` et modifier `Reader.h`, `Constants.h` : capacités, filtre de profil, signatures et contrat commun de lecture.
- [x] Diagnostics et intégration
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`), `src/MediaEngine/src/Images/ReadRejection.cpp` et créer les catalogues `Constants/MediaEngine/teledisk-reader.json` et `Languages/<langue>/MediaEngine/teledisk-reader.json` sous `src/Translations/Catalogs/` : erreurs de conteneur traduites et politique de rejet explicite.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler le lecteur, son assemblage sectoriel et son adapter.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-teledisk-reader/CMakeLists.txt` et `main.cpp` : contrôler commentaire, CRC, taille dominante, données manquantes, adresses/ordre enregistrés, classement, lecture avancée et sondes sur des conteneurs synthétiques.
  - [x] Supprimer `build/verification-teledisk-reader/` après consignation.
