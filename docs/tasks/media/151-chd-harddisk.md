# Profil disque dur CHD

- [x] Lecture et géométrie
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Chd/Constants.h`, `Container.h`, `Layout.cpp` : profil autonome V5 non compressé, carte, dimensions et géométrie GDDD conformes au C#.
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Chd/Reader.cpp` : reconnaissance et BlockImage avec plages stockées/zéro, géométrie et propriétés.
- [x] Écriture et intégration
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Chd/Writer.cpp` : sortie atomique V5 non compressée, hunks complétés, carte, SHA1 logique et métadonnées GDDD.
  - [x] Modifier `src/MediaEngine/src/Images/ReadRejection.cpp`, diagnostics et catalogues Constants/langues : rejets identifiés et erreurs traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : référencer les nouveaux fichiers.
- [x] Validation ciblée
  - [x] Créer `build/verification-chd-harddisk/CMakeLists.txt` et `main.cpp` : géométrie explicite, aller-retour, carte zéro/hunk final, SHA1, refus des profils incompatibles et annulation atomique.
  - [x] Supprimer `build/verification-chd-harddisk/` et ses artefacts après consignation.
