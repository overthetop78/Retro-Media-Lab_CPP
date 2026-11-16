# Lecteur 86F

- [x] Conteneur, pistes et représentation protégée
  - [x] Créer `src/MediaEngine/src/Formats/I86f/Constants.h`, `I86f.h`, `Reader.cpp`, `ProtectedImage.cpp` et `Adapter.cpp` : drapeaux de fichier/piste, table par face, bornes des pistes, cellules explicites et alignement des mots, permutation des octets, suppression des pistes sans transition, conversion protégée et lecteur du registre commun.
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`), `src/MediaEngine/src/Images/ReadRejection.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/86f.json` et ses équivalents `Languages/<langue>/MediaEngine/` : erreurs de structure et image vide traduites.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `scripts/translations/ignores/common.txt` et les projets/filtres Visual C++ générés : sources, constantes et invariant 86F.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-86f/CMakeLists.txt` et `main.cpp` : deux faces, cellules explicites, permutation des mots, cadence/flux, piste sans transition, métadonnées du registre et erreurs de table/piste.
  - [x] Supprimer `build/verification-86f/` et ses fichiers/binaires temporaires après consignation.
