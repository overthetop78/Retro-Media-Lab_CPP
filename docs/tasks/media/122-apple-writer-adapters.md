# Writers Apple : contrats et sémantique des formats

- [x] Validation et façade NIB/WOZ
  - [x] Créer `src/MediaEngine/src/Formats/Apple/Writing.h`, `Writing.cpp` et modifier `Constants.h` : validation du système de fichiers source/cible Apple, sélection sectorielle par extension et façade d’encodage NIB/WOZ avec limites propres à chaque conteneur.
- [x] Adapters communs
  - [x] Créer `src/MediaEngine/src/Formats/Apple/WriterAdapters.h` et `WriterAdapters.cpp` : exposer les writers Apple sectoriel, nibble, Macintosh et Lisa ; préserver les métadonnées DiskCopy et la sélection des backends du C#.
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`) et créer les catalogues `Constants/MediaEngine/apple-writer-adapters.json` et `Languages/<langue>/MediaEngine/apple-writer-adapters.json` sous `src/Translations/Catalogs/` : erreurs de sémantique Apple et d’extension NIB/WOZ traduites.
  - [x] Restaurer `src/Translations/Catalogs/Constants/MediaEngine/apple-writing.json` et `Languages/<langue>/MediaEngine/apple-writing.json` depuis les catalogues générés précédents : rétablir les clés préexistantes écrasées par collision de nom ; déplacer les nouveaux messages vers `apple-writer-adapters.json`.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger Apple, NIB et WOZ dans les nouveaux messages ; corriger tout ancien catalogue qui aurait traduit ces invariants.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler validation, façade et adapters.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-apple-writer-adapters/CMakeLists.txt` et `main.cpp` : vérifier les couples profil/extension, refus de changement de système de fichiers, sélection des layouts Macintosh/Lisa, sorties effectives NIB/WOZ et conservation des métadonnées DiskCopy lors d’une écriture par adapter.
  - [x] Supprimer `build/verification-apple-writer-adapters/` et sa sortie synthétique après consignation.
