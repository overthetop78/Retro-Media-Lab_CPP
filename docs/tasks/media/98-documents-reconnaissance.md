# Documents reconnus et contexte de lecture partagé

- [x] Messages et noms traduisibles
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/MessageRecipe.h` et modifier `MediaDocument.h`, `FileSystemModels.h`, `Volumes.h`, `src/MediaFileSystems/src/Partitioning/Volumes.cpp` et `src/MediaFileSystems/src/FileSystems/Cartridge/Readers.cpp` : conserver clés, arguments et domaine des diagnostics et noms virtuels, séparés des noms physiques lus sur les supports.
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaKind.h`, `MessageInfo.h` et `VolumeInfo.h` : vues publiques de métadonnées sans inclusion des contrats MFS dans App.
- [x] Source et document possédés
  - [x] Créer `src/MediaEngine/src/Images/MediaSource.h`, `MediaDocument.h` et `MediaDocument.cpp` : reprendre source/associations/paramètres/mécanique, document immuable, volumes et messages, et liaison mécanique des blocs déjà portée.
- [x] Contexte de reconnaissance
  - [x] Créer `src/MediaEngine/src/Images/RecognitionContext.h`, `RecognitionContext.cpp` et `Constants.h` : lectures bornées, cache concurrent de pages partageant succès/échecs, chargement complet limité, fabrique mécanique empruntée et conservation des sources jusqu'à la fin des futures.
  - [x] Créer `src/MediaEngine/src/Images/Readers.h` : contrat des lecteurs et leurs capacités, réutilisable par les adaptateurs de formats restants.
- [x] Intégration
  - [x] Modifier les diagnostics MFS, `src/MediaEngine/CMakeLists.txt` et `scripts/quality/audit_literals.py` : intégrer modèles, contexte et validations structurées des contrats.
  - [x] Créer les catalogues `MediaFileSystems/media-document.json` dans `src/Translations/Catalogs/Constants/` et `Languages/<langue>/` : traduire les validations et erreurs du contexte.

Les liaisons mécanique secteur/optique et les façades de service restantes ne sont pas déclarées complètes par ce bloc. Aucune nouvelle batterie comparative C#.

- [x] Primitives numériques réellement partagées
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/MetadataNumbers.h` et `MetadataNumberConstants.h`, modifier `src/MediaFileSystems/src/Primitives/Metadata.h` et `scripts/quality/audit_literals.py`, puis supprimer `src/MediaFileSystems/src/Primitives/NumericMetadataConstants.h` : exposer l'équivalent public C# des lectures byte/ushort/int et Integer/None, réutilisable par Engine sans dépendance aux en-têtes privés MFS.

- [x] Relecture linguistique restante
  - [x] Modifier les catalogues `src/Translations/Catalogs/Languages/<langue>/MediaEngine/amstrad-fds.json` et `MediaFileSystems/media-document.json` : relire les langues autres que le français et corriger les contresens éventuels ; les clés, paramètres et invariants sont déjà contrôlés automatiquement.

Le catalogue MediaFileSystems/media-document.json est relu dans les 29 langues au 17 novembre 2026 ; seule la relecture MediaEngine/amstrad-fds.json reste ouverte dans ce groupe. Voir docs/validation/media/relecture-blocs-documents.md.

La relecture amstrad-fds.json est également achevée au 17 novembre 2026 ; voir docs/validation/media/relecture-amstrad-fds.md.
