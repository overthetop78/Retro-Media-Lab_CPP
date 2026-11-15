# Systèmes de fichiers optiques et composition complète

- [x] Primitives optiques communes
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Optical/Common.h`, `Common.cpp` et `Constants.h` : sélection session/piste, conversions de texte et lectures bornées réutilisables, sans confondre adresses relatives et absolues.
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/OpticalReaders.h` : contrats des cinq lecteurs ISO9660/Joliet/Rock Ridge/UDF/XDVDFS.
- [x] Portage des lecteurs C#
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Iso9660/Constants.h`, `Internal.h`, `Reader.cpp`, `Names.cpp` : descripteurs, doubles endianness, arbre et dates, Joliet UCS-2 et Rock Ridge SUSP/NM/ER.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Xbox/Xdvdfs/Constants.h` et `Reader.cpp` : descripteur Xbox, arbre binaire, fichiers/répertoires et limites C# ; refuser les cycles entre répertoires pour éviter une récursion non bornée.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Udf/Constants.h`, `Internal.h`, `Descriptors.cpp`, `Reader.cpp` et `Entries.cpp` : tags/CRC, Unicode CS0, partitions, allocations, arbre et dates selon le C#.
- [x] Intégration
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaFileSystems/src/Diagnostics.cpp`, `src/MediaFileSystems/CMakeLists.txt` et `scripts/quality/audit_literals.py` : intégrer sources, constantes et diagnostics traduisibles précis.
  - [x] Créer les catalogues `MediaFileSystems/optical-filesystems.json` dans `src/Translations/Catalogs/Constants/` et `Languages/<langue>/` : erreurs et avertissements des lecteurs optiques.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/MediaExplorer.h` et `src/MediaFileSystems/src/Exploration/MediaExplorer.cpp` : fournir la composition complète C# : 21 lecteurs sectoriels, adaptateurs fournis, puis neuf lecteurs non sectoriels, dans le même ordre.

Validation : une compilation commune en fin de bloc ; contrôles ciblés sur le raccordement, sans rejouer les batteries sectorielles déjà validées.

- [x] Invariants des nouveaux catalogues
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger ISO 9660, Joliet, Rock Ridge, XDVDFS, UDF, NSR et ICB dans les messages avant génération Argos.

- [x] Contrôle ciblé de l'intégration nouvelle
  - [x] Modifier `build/verification-volume-contract/main.cpp` : ajouter uniquement un mode optique en mémoire, un volume minimal par lecteur et la vérification des 30 lecteurs ordonnés ; ne pas relancer les comparaisons de secteurs/constructeurs antérieures.
  - [x] Supprimer `build/verification-volume-contract/` après conservation des résultats utiles du contrôle optique et des contrôles de contrats déjà effectués.

- [x] Solution et bibliothèques de langues
  - [x] Modifier `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` avec `scripts/local-building/update-solution.py` : faire apparaître les nouvelles sources et catalogues dans la solution Visual C++ existante.
  - [x] Modifier les DLL générées `build/debug/bin/` des cibles `RetroMediaLab.Catalog.*` : compiler ensemble les trente catalogues après stabilisation des JSON, sans nouvelle génération Argos des catalogues déjà relus.
- [ ] Relecture linguistique restante
  - [ ] Modifier les catalogues `src/Translations/Catalogs/Languages/<langue>/MediaFileSystems/optical-filesystems.json` : poursuivre la relecture des langues autres que le français et corriger les contresens détectés.
