# Progression des écritures par unités de visualisation

- [x] Contrats et parcours
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaExplorationProgress.h` et modifier `MediaExploration.h` : partager la progression sans dépendance au résultat disque et permettre une recette traduisible de détail.
  - [x] Créer `src/MediaEngine/src/Images/WritingProgress.h`, `WritingProgress.cpp` et `WritingProgressConstants.h` : registre de visualisation, progression 60–95, parcours des pistes/éléments et observation des secteurs ; indexer les adresses physiques une fois pour éviter un balayage complet par piste.
  - [x] Modifier `src/MediaEngine/src/Images/WritingService.h` et `WritingService.cpp` : surcharge avec progression, conserver la surcharge existante et les sorties/nettoyages ; reporter les unités avant le writer comme dans le C#.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : intégrer le bloc.
- [x] Vérification ciblée
  - [x] Créer `build/verification-writing-progress/CMakeLists.txt` et `main.cpp` : ordre des notifications avant le writer, parcours sectoriel et flux, absence de parcours sans callback, descripteur vide, annulation et exception de callback avant toute écriture.
  - [x] Supprimer `build/verification-writing-progress/` et ses artefacts après utilisation.
