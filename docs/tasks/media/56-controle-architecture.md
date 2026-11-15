# Contrôle transversal des bibliothèques médias

- [x] Frontières des composants
  - [x] Créer `scripts/quality/audit_media_architecture.py` : contrôler les inclusions de toutes les sources présentes, les dépendances CMake, les accès privés entre composants et l’absence de GUI/API hôte dans les trois bibliothèques médias ; produire un résultat vérifiable.

- [x] Rangement de la compilation
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `src/MediaFileSystems/CMakeLists.txt`, `src/MediaAnalysis/CMakeLists.txt` : regrouper explicitement les sources par responsabilité et sous-famille, en conservant exactement les sources compilées et les dépendances.
