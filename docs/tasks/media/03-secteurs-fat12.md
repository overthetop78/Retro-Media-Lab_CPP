# Secteurs, exploration et famille FAT12 complète

Bloc du portage complet. Inclut la lecture FAT12, les variantes du catalogue, les primitives réutilisables et la construction sectorielle pour la migration. Aucun matériel physique n’est écrit.

- [x] Contrats et modèles
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/SectorImage.h`, `FileSystemModels.h`, `Migration.h`, `Fat.h` et `Fat12.h` : blocs et métadonnées sectorielles, interface sectorielle, dates, entrées/volumes, identifiants d’analyse, plan de migration et API FAT12 lecture/écriture/géométrie/noms.
  - [x] Créer `src/MediaFileSystems/src/Primitives/Timestamps.cpp` et `MetadataConstants.h` : validation et décodage des dates indépendants du système hôte, constantes techniques communes.
  - [x] Créer `src/MediaEngine/src/Models/Sectors/SectorImages.h`, `SectorImages.cpp`, `SectorConstants.h` : images sectorielles propriétaires validées, secteurs manquants/tronqués, métadonnées enregistrées et accès paresseux depuis une source bornée.
- [x] Primitives et catalogue FAT12
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Fat/Constants.h` et `BootAndNames.cpp` : primitives FAT communes pour BPB, noms, labels et dates, séparées des règles FAT12.
  - [x] Réorganiser `src/MediaFileSystems/src/Partitioning/Binary.h` en `src/MediaFileSystems/src/Primitives/Binary.h`, `BinaryConstants.h` et `src/MediaFileSystems/src/Partitioning/Helpers.h` ; modifier `Mbr.cpp`, `Gpt.cpp` et `PartitionConstants.h` : partager réellement les lectures/écritures little endian, CRC et GUID sans copier leurs implémentations.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Fat/Fat12/Constants.h`, `FormatCatalog.h`, `Fat12Internal.h` et `Layout.cpp` : constantes nommées, associations des 28 formats C#, BPB, cinq dispositions historiques, DOS Plus 800 et géométrie FAT.
- [x] Lecture et exploration
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Fat/Fat12/Tables.cpp`, `Directory.cpp` et `Reader.cpp` : copies de FAT, secteurs et chaînes de clusters, noms/labels/dates, répertoires, fichiers, validité, espace libre et diagnostics.
  - [x] Créer `src/MediaFileSystems/src/Exploration/EntryAnalysis.cpp` et `AnalysisConstants.h` : résolution de famille, appel réel à MediaAnalysis et résultats indépendants du GUI.
  - [x] Créer `src/MediaEngine/src/Exploration/SectorExploration.h` et `SectorExploration.cpp` : trajet interne source/image sectorielle → lecture FAT12 → analyse des entrées, sans exposer les API MFS à App.
- [x] Écriture FAT12
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Fat/Fat12/Writer.cpp` : politiques de noms/label, allocation, BPB, FAT, répertoires, dates et plan sectoriel complet repris du writer C#, sans accès direct au matériel.
- [x] Construction et traduction
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h` et `src/MediaFileSystems/src/Diagnostics.cpp` : diagnostics de secteurs/FAT12 à codes et paramètres.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/fat12.json` et les fichiers équivalents des 29 langues : base anglaise, Argos, invariants/paramètres préservés et relecture.
  - [x] Modifier `scripts/translations/ignores/common.txt` si des invariants techniques manquent ; modifier `scripts/quality/audit_literals.py` : recenser explicitement les nouveaux fichiers de constantes.
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt` et `src/MediaEngine/CMakeLists.txt` ; régénérer `RetroMediaLab.vcxproj` et `.filters` par le générateur existant : construire les modèles, lecteur, writer et pont d’exploration dans les DLL appropriées.
- [x] Validation comparative
  - [x] Créer `build/verification-fat12/` : banc temporaire C#/C++ regroupé, variantes BPB/historiques/DOS Plus/Apricot, FAT et répertoires dégradés, lecture/écriture/réouverture, métadonnées, analyse, annulation et limites.
  - [x] Supprimer `build/verification-fat12/` et tous ses artefacts temporaires après validation ; ne conserver que les résultats documentés sous docs/validation/media/.
