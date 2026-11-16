# Métadonnées de disques et captures

- [x] Contrats et lecture légère SCP
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/ScpHeader.h`, `ScpHeaderConstants.h` et `ScpCaptureInfo.h`, modifier `src/MediaEngine/src/Formats/Scp/Scp.h` et `Constants.h` : partager l'en-tête public sans duplication et les constantes de lecture.
  - [x] Créer `src/MediaEngine/src/Formats/Scp/CaptureInfo.cpp` : en-tête/table, décompte des pistes et checksum incrémental sans décodage de flux.
- [x] Métadonnées techniques des images
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/DiskMetadata.h`, `DiskMetadataIds.h`, `src/MediaEngine/src/Exploration/Metadata/Metadata.h`, `Constants.h`, `Identification.cpp` et `Content.cpp` : identifiants de systèmes/protection, regroupement ordonné, chargeur Amiga, archives ATN, organisation sans catalogue et signatures de modification suivant le C#.
  - [x] Modifier `src/MediaEngine/src/Exploration/Disk/Explorer.h` et `Explorer.cpp` : produire les métadonnées sur l'image principale avec les formats détectés, les conserver à la sélection et propager l'annulation.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : intégrer les contrats et sources.
- [x] Vérification ciblée
  - [x] Créer `build/verification-disk-metadata/CMakeLists.txt` et `main.cpp` : identifiants, signatures et seuils, somme de contrôle du chargeur Amiga, archives ATN et lecture SCP par blocs avec checksum et table partielle ; compiler le bloc.
  - [x] Supprimer `build/verification-disk-metadata/` et ses fichiers temporaires après utilisation.
