# Lecteur SNATCH-IT CP2

- [x] Conteneur et reconstruction
  - [x] Créer `src/MediaEngine/src/Formats/Cp2/Constants.h`, `Cp2.h`, `Layout.cpp`, `Reader.cpp` et `Adapter.cpp` : groupes et descripteurs, ordre angulaire stable, exclusion des enregistrements synthétiques incohérents, consommation des charges utiles de toutes tailles, conservation du premier secteur 512 octets par adresse, reconstruction IBM et lecteur du registre commun.
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`), `src/MediaEngine/src/Images/ReadRejection.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/cp2.json` et ses équivalents `Languages/<langue>/MediaEngine/` : sept erreurs paramétrées traduites.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `scripts/translations/ignores/common.txt` et les projets/filtres Visual C++ générés : sources, constantes et invariants CP2/SNATCH-IT.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-cp2/CMakeLists.txt` et `main.cpp` : groupes enchaînés, ordre angulaire, doublons, secteurs synthétiques et tailles ignorées, secteurs manquants zéro remplis, reconnaissance du registre et rejets des structures invalides.
  - [x] Supprimer `build/verification-cp2/` et ses fichiers/binaires temporaires après consignation.
