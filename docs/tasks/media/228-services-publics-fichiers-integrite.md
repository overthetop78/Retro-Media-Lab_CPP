# Services publics de fichiers et d'intégrité

- [x] Services sans état
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaUtilities.h` et `src/MediaEngine/src/Services/Utilities.cpp` : exposer le CRC32 WOZ et le parseur d'entiers de métadonnées existants par MediaEngine, sans initialiser la composition globale.
- [x] Parcours différé et emprunté
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FileTreeCursor.h` et `src/MediaFileSystems/src/Exploration/FileTreeCursor.cpp` : curseur itératif profondeur d'abord, racine incluse si fichier, accès différé aux enfants et vues empruntées sans matérialisation des descendants.
  - [x] Modifier `src/MediaFileSystems/src/Exploration/MediaExplorer.cpp` : réutiliser ce curseur pour rechercher le premier fichier.
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaFileTraversal.h` et `src/MediaEngine/src/Services/FileTraversal.cpp` : range C++ typé différé, pont opaque vers le curseur MFS, conservation des callbacks et absence de dépendance publique MFS.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `src/MediaFileSystems/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `.filters` : intégrer le bloc.
- [x] Vérification ciblée
  - [x] Créer `build/verification-media-utilities/CMakeLists.txt` et `main.cpp` : CRC et lecture numérique via les services existants, ordre/différé/racine-fichier, collections empruntées, arrêt anticipé, exceptions des callbacks et arbre profond sans récursion.
  - [x] Supprimer `build/verification-media-utilities/` et ses artefacts après utilisation.
