# Runtimes des images Flux et MFM

- [x] Séparation des runtimes derrière le contrat public
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/RuntimeDisk.h`, `src/MediaEngine/src/Runtime/DiskState.h`, `Disk.cpp`, `Sectors.cpp`, `Persistence.cpp`, `Metadata.cpp`, `Tracks.cpp`, `WinchesterTracks.cpp`, `FormatTrack.cpp` et créer `DecodedDisk.cpp`, `PhysicalStates.h` : séparer le contrat interne commun et le runtime secteur/bloc existant, partager fermeture et propriété des mécaniques, sélectionner le runtime après reconnaissance.
  - [x] Créer `src/MediaEngine/src/Runtime/FluxDisk.cpp` : conserver les révolutions des conteneurs de flux, laisser les pistes absentes sans impulsions, refuser les écritures de régions faibles, remplacer les fenêtres sur toutes les révolutions et publier via les écrivains existants.
  - [x] Créer `src/MediaEngine/src/Runtime/MfmDisk.cpp` et modifier `src/MediaEngine/src/Formats/HardDisk/MfmEmu/Representation.h` : runtime MFM utilisant la représentation physique et son lecteur sectoriel Adaptec existants, cache de piste, métadonnées physiques et publication avec réouverture.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : intégrer les runtimes séparés.
- [x] Vérification ciblée
  - [x] Créer `build/verification-physical-backends/CMakeLists.txt`, `main.cpp` et images temporaires : sélection des trois runtimes, révolutions multiples et piste absente, conservation des pistes non ciblées, fenêtres et secteurs MFM, sauvegarde/relecture, protection et échec de publication, propriété des mécaniques.
  - [x] Supprimer `build/verification-physical-backends/` avec ses images et exécutables après utilisation.

