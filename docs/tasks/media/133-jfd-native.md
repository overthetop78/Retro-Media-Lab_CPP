# Structure native JFD

- [x] Décodage et modification sans perte de la structure native
  - [x] Créer `src/MediaEngine/src/Formats/Jfd/Constants.h`, `Jfd.h`, `Binary.h`, `Decode.cpp`, `Rewrite.cpp` et `Compression.cpp` : tables principales et delta, secteurs absents, pistes brutes conservées, intégrité, détection des modifications incompatibles et gzip limité avec contrôle CRC par zlib (déjà intégrée avec libarchive) ; ressources libérées par RAII.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h` et `src/MediaEngine/src/Diagnostics.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/jfd-native.json` et ses équivalents `Languages/<langue>/MediaEngine/jfd-native.json` : diagnostics traduisibles du codec natif.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py` et les projets/filtres Visual C++ générés : compiler les sources JFD et réutiliser zlib déjà embarquée avec libarchive.
- [x] Vérification ciblée
  - [x] Créer `build/verification-jfd-native/CMakeLists.txt` et `main.cpp` : vérifier tables delta, conservation des pistes brutes et stubs, édition compatible et rejet des recouvrements contradictoires, décompression gzip bornée.
  - [x] Supprimer `build/verification-jfd-native/` et ses fichiers temporaires après consignation.

La création d'une nouvelle image et les adaptateurs des registres seront portés ensuite ; ce bloc ne constitue pas l'intégration complète de JFD.
