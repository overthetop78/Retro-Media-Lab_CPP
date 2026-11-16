# Lecteur KryoFlux

- [x] Décodage et organisation
  - [x] Créer `src/MediaEngine/src/Formats/KryoFlux/Constants.h`, `Stream.h`, `Internal.h`, `Metadata.cpp`, `Decoding.cpp` : opcodes, lecture des blocs hors bande, horloge, métadonnées et découpage en révolutions, conformément au lecteur C#.
  - [x] Créer `src/MediaEngine/src/Formats/KryoFlux/Files.cpp`, `Adapter.h`, `Adapter.cpp` : reconnaissance, fichiers physiques associés et document flux dans le registre existant.
  - [x] Modifier les diagnostics MediaEngine et créer `src/Translations/Catalogs/Constants/MediaEngine/kryoflux.json` et `Languages/<langue>/MediaEngine/kryoflux.json` : erreurs traduites ; modifier `scripts/translations/ignores/common.txt` pour les invariants techniques si nécessaire.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : compiler et référencer les nouveaux fichiers et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-kryoflux-reader/CMakeLists.txt` et `main.cpp` : opcodes, horloge, index, métadonnées, positions invalides, troncature, regroupement multipiste et annulation.
  - [x] Supprimer `build/verification-kryoflux-reader/` avec ses données synthétiques et binaires après consignation.
