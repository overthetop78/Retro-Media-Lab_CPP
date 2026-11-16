# Contrats publics d'exploration des disques

- [x] Contrats publics possédés
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/DiskExplorationTypes.h` et `DiskExploration.h`, modifier `ExplorationInfo.h` et `src/MediaEngine/src/Exploration/Scp/Inspection.h` : instantané de conteneur/pistes/révolutions/secteurs/formats, réutilisation des entrées/volumes/diagnostics existants, progression publique et sélection opaque sans MFS dans App.
- [x] Projection et façade
  - [x] Créer `src/MediaEngine/src/Exploration/Disk/Projection.h`, `Projection.cpp`, `ProjectionTracks.cpp`, `ProjectionFormats.cpp` et `ProjectionConstants.h` : métadonnées du conteneur, secteurs manquants, formats et volumes, volume physique de repli sans catalogue, diagnostics, conservation des octets et des captures par propriété partagée immutable.
  - [x] Créer `src/MediaEngine/src/Exploration/Disk/Facade.cpp` : explorer un chemin ou un document et sélectionner un format sans nouvelle reconnaissance.
  - [x] Modifier `src/MediaEngine/src/Images/ExplorationMapper.h` et `ExplorationMapper.cpp` : réutiliser le mapper de volumes et d'entrées avec l'inférence de validité propre au contrat disque C#.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : intégrer les sources et contrats ; compiler les cibles dépendantes concernées.
- [x] Vérification ciblée
  - [x] Créer `build/verification-disk-contracts/CMakeLists.txt` et `main.cpp` : projection sectorielle/SCP, champs des volumes/entrées, diagnostics, secteurs absents/invalides, compteurs et sélection possédée ; vérifier la façade par chemin sur une petite image synthétique.
  - [x] Supprimer `build/verification-disk-contracts/` et ses fichiers temporaires après utilisation.
