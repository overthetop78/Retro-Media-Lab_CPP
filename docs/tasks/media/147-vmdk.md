# VMDK monolithique

- [x] Lecture et validation
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Vmdk/Constants.h`, `Container.h`, `Descriptor.h`, `Descriptor.cpp` : constantes, contrats et analyse des descripteurs C# avec une étendue FLAT autonome.
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Vmdk/Sparse.h`, `Sparse.cpp`, `Reader.cpp` : validation du conteneur sparse, grain directory/tables, plages logiques, métadonnées et reconnaissance.
- [x] Écriture et raccordement
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Vmdk/Writer.cpp`, `SparseWriter.cpp`, `Writing.h` : création du descripteur, variantes Fixed/Sparse, publication de la paire FLAT ou du fichier sparse par les transactions communes.
  - [x] Modifier les diagnostics MediaEngine et `src/MediaEngine/src/Images/ReadRejection.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/vmdk.json` et `Languages/<langue>/MediaEngine/vmdk.json` : erreurs traduites et invariants communs.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : compiler et référencer les nouveaux fichiers et constantes.
- [x] Validation ciblée
  - [x] Créer `build/verification-vmdk/CMakeLists.txt` et `main.cpp` : paire FLAT avec décalage, sparse avec zéros et dernier grain partiel, variantes, rejets des descripteurs/tables et annulation de publication.
  - [x] Supprimer `build/verification-vmdk/` avec ses fichiers synthétiques et binaires après consignation.
