# Exploration Famicom Disk System

- [x] Lecteur et primitives
  - [x] Créer `src/MediaFileSystems/src/Primitives/Metadata.h` et modifier `src/MediaFileSystems/src/FileSystems/Cartridge/Readers.cpp` : partager la lecture décimale `NumberStyles.None` des métadonnées, sans signe ni espaces.
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FamicomDisk.h`, `src/MediaFileSystems/src/FileSystems/Nintendo/FamicomDisk/Constants.h` et `Reader.cpp` : reprendre les faces, blocs HVC, noms ASCII, fichiers partiellement récupérés et avertissements C#.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FormatIds.h`, `Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaFileSystems/src/Diagnostics.cpp`, `src/MediaFileSystems/CMakeLists.txt` et `scripts/quality/audit_literals.py` : intégrer identifiant, constantes, erreurs et lecteur.
  - [x] Créer les catalogues `MediaFileSystems/famicom-disk.json` dans `src/Translations/Catalogs/Constants/` et `Languages/<langue>/` : traduire erreurs et avertissements et préserver le nom invariant Famicom Disk System.

Validation : compilation incrémentale ; conserver les fichiers déjà lus lorsqu'une face comporte ensuite une erreur, comme dans le C#.

- [x] Constantes de la primitive partagée
  - [x] Restaurer `src/MediaFileSystems/src/Primitives/MetadataConstants.h` depuis le contenu original des journaux de session et créer `src/MediaFileSystems/src/Primitives/NumericMetadataConstants.h` et modifier `Metadata.h` et `scripts/quality/audit_literals.py` : séparer les constantes lexicales de l'implémentation du parseur.

- [x] Protection des invariants
  - [x] Modifier `scripts/translations/ignores/common.txt` : ajouter Famicom Disk System et Nintendo GameCube FST aux noms invariants communs avant traduction.

- [x] Relecture linguistique restante
  - [x] Modifier les catalogues `src/Translations/Catalogs/Languages/<langue>/MediaFileSystems/` du présent bloc : corriger les contresens restants après lecture des langues non encore relues.

Relecture des trois catalogues dans les 29 langues effectuée le 17 novembre 2026 ; corrections et limites dans `docs/validation/media/relecture-cartouches-fds-gamecube.md`.
