# Raccordement ADFS aux volumes par blocs

- [x] Raccorder la vue ADFS déjà portée à l'exploration commune
  - [x] Créer `src/MediaFileSystems/src/Exploration/Composition/AdfsDocument.h` et `AdfsDocument.cpp` : adaptateur de lecteur ADFS acceptant les volumes sectoriels et les volumes par blocs, lecture à la demande et limites vérifiées par la vue existante.
  - [x] Modifier `src/MediaFileSystems/src/Exploration/Composition/Catalog.cpp` : enregistrer cet adaptateur à la place du lecteur sectoriel générique ADFS.
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `.filters` : intégrer l'adaptateur.
- [x] Vérifier uniquement le raccordement des volumes
  - [x] Créer `build/verification-adfs-document/CMakeLists.txt` et `main.cpp` : explorer un volume ADFS temporaire en mémoire à une adresse non nulle, vérifier les limites et l'annulation sans retester l'algorithme ADFS.
  - [x] Supprimer `build/verification-adfs-document/` après utilisation.
