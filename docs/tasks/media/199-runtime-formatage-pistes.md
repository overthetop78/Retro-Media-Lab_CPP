# Remplacement et projection des champs de piste

- [x] Secteurs physiques et formatage runtime
  - [x] Créer `src/MediaEngine/src/Runtime/SectorTracks.h` et `SectorTracks.cpp` : porter Validate, Replace et Project depuis SectorTrackFunctions C#, préserver adresses enregistrées, ordre, diagnostics et géométrie, refuser la perte de champs non représentables.
  - [x] Créer `src/MediaEngine/src/Runtime/FormatTrack.cpp` et modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/RuntimeDisk.h` : formatage sectoriel avec reconstruction des descripteurs ImageDisk et conservation des pistes non ciblées.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compiler le formatage et les fonctions de piste.
- [x] Vérification ciblée
  - [x] Créer `build/verification-sector-tracks/CMakeLists.txt` et `main.cpp` : ordre physique et logique, adresses enregistrées, géométrie, champs répétés et manquants, refus sans changement de l’état, sauvegarde et relecture ImageDisk/CPC/IBM.
  - [x] Supprimer `build/verification-sector-tracks/` et ses images temporaires après utilisation.

