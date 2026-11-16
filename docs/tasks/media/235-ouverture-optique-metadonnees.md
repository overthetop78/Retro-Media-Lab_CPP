# Ouverture optique et constantes partagées

- [x] Terminer le point d'entrée des sessions optiques
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/OpticalDiscSession.h` et `src/MediaEngine/src/Images/Optical/DiscSession.cpp`, créer `src/MediaEngine/src/Images/Optical/SessionOpening.cpp` : ouverture par la composition commune, observation mécanique désactivée pendant la lecture, option d'audio compagnon, transfert de propriété et nettoyage unique sur exception.
- [x] Unifier les métadonnées de conteneur
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FamicomDiskMetadataConstants.h` : sept clés communes de métadonnées FDS.
  - [x] Modifier `src/MediaEngine/src/Formats/Cartridge/Constants.h` et `src/MediaFileSystems/src/FileSystems/Nintendo/FamicomDisk/Constants.h` : utiliser les mêmes clés partagées.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FormatIds.h` : compléter les identifiants Nintendo Wii U et GDI présents dans le C#.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : intégrer le point d'entrée et les constantes.
- [x] Vérification du raccordement
  - [x] Créer `build/verification-optical-opening/CMakeLists.txt` et `main.cpp` : ouverture depuis un chemin, lecture de secteur, audio compagnon optionnel et annulation, sans retester les décodeurs optiques.
  - [x] Supprimer `build/verification-optical-opening/` et tous ses médias temporaires après utilisation.
