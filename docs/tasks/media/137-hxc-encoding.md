# Codage natif HxC Stream

- [x] Captures et métadonnées
  - [x] Créer `src/MediaEngine/src/Formats/HxcStream/Constants.h`, `Stream.h`, `Internal.h`, `Encoding.cpp`, `Blocks.cpp` et `Digest.cpp` : résolution exacte 40/20 ns, impulsions et index échantillonnés, blocs LZ4, CRC des chunks, clés par piste et empreinte SHA256 compatible avec le C#.
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`), créer `src/Translations/Catalogs/Constants/MediaEngine/hxc-encoding.json` et ses équivalents `Languages/<langue>/MediaEngine/` : sept erreurs de codage traduites.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `scripts/translations/ignores/common.txt` et les projets/filtres Visual C++ générés : sources, constantes et dépendance LZ4 déjà embarquée.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-hxc-encoding/CMakeLists.txt` et `main.cpp` : empreinte binaire, résolution exacte, impulsions/index, métadonnées filtrées, CRC et décompression des blocs LZ4 produits, erreurs de représentation et limites.
  - [x] Supprimer `build/verification-hxc-encoding/` et ses fichiers/binaires temporaires après consignation.

Le lecteur, la récupération des captures natives et le writer multipiste ne sont pas déclarés terminés par ce bloc.
