# Sources bornées et partitions — MediaEngine / MediaFileSystems

Bloc du portage complet ; les autres représentations, systèmes de fichiers, formats, conversions et services restent à porter.

- [x] Contrats et sources
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Export.h`, `Diagnostics.h`, `DiagnosticKeys.h`, `Sources.h`, `Volumes.h`, `VolumeConstants.h` : exports, diagnostics à codes/arguments traduisibles, source bornée avec annulation, média par blocs et descripteurs de volumes, API sans contrôles graphiques.
  - [x] Créer `src/MediaFileSystems/src/Diagnostics.cpp` et `Sources.cpp` : clés de diagnostic, exceptions et validation des accès aux blocs.
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/Export.h` et `Sources.h`, `src/MediaEngine/src/Sources/SourceConstants.h` et `RandomAccessSources.cpp` : sources fichier et mémoire propriétaires, lectures exactes, bornes et annulation, nettoyage RAII.
  - [x] Déplacer `src/MediaEngine/include/RetroMediaLab/MediaEngine/Sources.h` vers `src/MediaEngine/src/Sources/RandomAccessSources.h` et modifier ses includes ainsi que `src/MediaEngine/CMakeLists.txt` : les sources et contrats MFS restent internes à MediaEngine, sans dépendance MFS publique transitive vers App.
- [x] Détecteurs de partitions
  - [x] Créer `src/MediaFileSystems/src/Partitioning/PartitionConstants.h` et `Binary.h` : offsets nommés, lecture little endian bornée, CRC32 GPT, GUID et noms Unicode.
  - [x] Créer `src/MediaFileSystems/src/Partitioning/Mbr.cpp` : partitions primaires et chaîne EBR, limites, boucles, plages et chevauchements avec diagnostics.
  - [x] Créer `src/MediaFileSystems/src/Partitioning/Gpt.cpp` : entêtes primaire/secondaire, tableaux, CRC, identifiants, noms et limites.
  - [x] Créer `src/MediaFileSystems/src/Partitioning/Volumes.cpp` : sélection GPT/MBR et volume direct, sans déduire le système de fichiers de l’hôte.
- [x] Construction et textes
  - [x] Modifier `scripts/translations/translate.py` : traduire d’abord les phrases complètes si les invariants restent identiques, avant les protections de secours ; préciser le contexte « storage medium » dans les nouveaux diagnostics anglais et vérifier les traductions générées.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger les invariants MBR, EBR, GPT, CRC et LBA communs aux diagnostics de toutes les langues.
  - [x] Créer `src/MediaFileSystems/CMakeLists.txt` et `src/MediaEngine/CMakeLists.txt` ; modifier `CMakeLists.txt` : construire/installer les deux DLL, MediaEngine → MediaFileSystems → MediaAnalysis sans accès d’App aux composants internes.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/diagnostics.json` et `src/Translations/Catalogs/Languages/<langue>/MediaFileSystems/diagnostics.json` pour les 29 langues existantes : diagnostics C# et erreurs de sources via Argos, paramètres préservés et contrôle des traductions.
  - [x] Modifier `scripts/quality/audit_literals.py` et `scripts/local-building/update-solution.py` ; régénérer `RetroMediaLab.vcxproj` et `.filters` : constantes recensées explicitement, en-têtes des nouveaux composants visibles dans la solution.
- [x] Validation
  - [x] Créer `build/verification-partitions/` : banc temporaire C#/C++ unique avec images MBR/EBR/GPT générées, corruption, bornes, annulation et sources fichiers reçues/générées seulement dans ce dossier.
  - [x] Supprimer `build/verification-partitions/` et tous ses artefacts temporaires après validation, sans supprimer les fichiers préexistants.
