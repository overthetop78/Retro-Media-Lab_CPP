# Readers XFD, Atom DSK et IMG général

- [x] Conteneurs
  - [x] Créer `src/MediaEngine/src/Formats/Xfd/Reader.h`, `Reader.cpp` et `Constants.h` : quatre profils Atari, secteurs de boot courts et padding final nul de 16 octets uniquement.
  - [x] Créer `src/MediaEngine/src/Formats/AcornAtom/Reader.h`, `Reader.cpp` et `Constants.h` : profil 40 pistes, validation du catalogue Atom et document sectoriel commun.
  - [x] Créer `src/MediaEngine/src/Formats/Raw/ImgReader.h` et `ImgReader.cpp` : lecture IBM puis réidentification Amstrad CPC/PCW uniquement sans BPB FAT, en réutilisant les sondes MFS.
- [x] Diagnostics et intégration
  - [x] Modifier les diagnostics Engine et `src/MediaEngine/src/Images/ReadRejection.cpp`, créer les catalogues `src/Translations/Catalogs/Constants/MediaEngine/xfd-atom.json` et `Languages/<langue>/MediaEngine/xfd-atom.json` : traduire les trois rejets de conteneur.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` et `scripts/quality/audit_literals.py` : compiler les readers et déclarer les constantes ; régénérer les projets Visual C++.
- [x] Vérification ciblée
  - [x] Créer `build/verification-xfd-atom-img/CMakeLists.txt` et `main.cpp` : vérifier les longueurs/padding XFD, la numérotation Atom, le filtrage des readers et la conservation du profil IBM en présence d’un BPB.
  - [x] Supprimer `build/verification-xfd-atom-img/` après consignation du résultat.
