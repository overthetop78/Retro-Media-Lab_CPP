# Ouverture et exploration communes des médias

- [x] Contrats d'ouverture
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaExploration.h` : progression technique, résultat d'ouverture avec document opaque et explorations optionnelles ; API d'exploration et d'analyse.
  - [x] Modifier `DiskExplorationTypes.h`, `src/MediaEngine/src/Exploration/Disk/Projection.h` et `ProjectionTracks.cpp` : capture SCP publique possédée pour la notification de chargement, sans exposer les modèles internes.
- [x] Services partagés
  - [x] Modifier `src/MediaEngine/src/Images/ReadRejection.h`, `ReadRejection.cpp` et `src/MediaEngine/src/Exploration/Disk/Explorer.cpp` : partager la classification des formats non pris en charge utilisée par les replis de lecture.
  - [x] Créer `src/MediaEngine/src/Exploration/Media/Service.h`, `Service.cpp`, `Constants.h` et `Facade.cpp` : lecture avec repli de l'indication de format, indexation des bandes, exploration commune ; ouverture unique puis parcours disque et média concurrents, réemploi des secteurs décodés pour les sources de flux, annulation et exceptions conservées.
  - [x] Modifier `src/MediaEngine/src/Composition/Runtime.h`, `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : assembler les services avec les instances existantes et intégrer les fichiers.
- [x] Vérification ciblée
  - [x] Créer `build/verification-media-opening/CMakeLists.txt` et `main.cpp` : lecture unique, repli du format, progression, mode sans systèmes de fichiers, réemploi des secteurs de flux, capture possédée et annulation ; vérifier les résultats des parcours public disque et média avec une image temporaire.
  - [x] Supprimer `build/verification-media-opening/` et ses artefacts après utilisation.
