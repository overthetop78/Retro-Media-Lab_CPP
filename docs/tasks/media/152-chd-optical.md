# Lecture CHD optique

- [x] Sources et métadonnées
  - [x] Créer `src/MediaEngine/src/Formats/Common/Chd/Sources.h` et `Sources.cpp` : source CHD décodée avec cache sérialisé et source v5 brute par carte, lectures bornées et annulation.
  - [x] Créer `src/MediaEngine/src/Formats/Optical/Chd/Constants.h`, `Reader.h`, `Metadata.cpp` et `Layout.cpp` : profils bruts/décodés, métadonnées CHTR/CHT2/DVD et déclarations des pistes.
- [x] Pistes et reconnaissance
  - [x] Créer `src/MediaEngine/src/Formats/Optical/Chd/Reader.cpp` : pistes CD/DVD, fenêtres utilisateur, pregaps/index, sous-canaux, propriétés et reconnaissance conformes au C#.
  - [x] Modifier les diagnostics et `src/MediaEngine/src/Images/ReadRejection.cpp`, créer les catalogues Constants/langues : erreurs optiques traduisibles et rejets.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : référencer les nouveaux fichiers.
- [x] Validation ciblée
  - [x] Créer `build/verification-chd-optical/CMakeLists.txt` et `main.cpp` : CD/DVD, pistes triées/alignées, pregaps/sous-canaux, données compressées, cache, annulation et rejets.
  - [x] Supprimer `build/verification-chd-optical/` et ses artefacts après consignation.
