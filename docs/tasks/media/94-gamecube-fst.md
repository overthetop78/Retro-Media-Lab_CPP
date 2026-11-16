# Exploration Nintendo GameCube FST

- [x] Lecteur optique FST
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/GameCubeFst.h`, `src/MediaFileSystems/src/FileSystems/Nintendo/GameCube/Constants.h` et `Reader.cpp` : reprendre la sélection de piste, les bornes FST, les noms UTF-8, le parcours hiérarchique, les métadonnées et la limite de chargement C# de 64 Mio ; conserver les catégories d'exceptions filtrées par CanRead.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FormatIds.h`, `Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaFileSystems/src/Diagnostics.cpp`, `src/MediaFileSystems/CMakeLists.txt` et `scripts/quality/audit_literals.py` : intégrer le lecteur et ses constantes/diagnostics sans dépendance à App.
  - [x] Créer les catalogues `MediaFileSystems/gamecube-fst.json` dans `src/Translations/Catalogs/Constants/` et `Languages/<langue>/` : erreurs et avertissement de contenu trop grand traduisibles.

Validation : compilation et contrôle ciblé du parcours/bornage ; aucune nouvelle batterie comparative.

- [x] Relecture linguistique restante
  - [x] Modifier les catalogues `src/Translations/Catalogs/Languages/<langue>/MediaFileSystems/` du présent bloc : corriger les contresens restants après lecture des langues non encore relues.

Relecture des trois catalogues dans les 29 langues effectuée le 17 novembre 2026 ; corrections et limites dans `docs/validation/media/relecture-cartouches-fds-gamecube.md`.
