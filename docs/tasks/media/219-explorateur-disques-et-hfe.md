# Explorateur technique de disques et conversion HFE

- [x] Service commun d'exploration
  - [x] Créer `src/MediaEngine/src/Exploration/Disk/Explorer.h`, `Explorer.cpp` et `Constants.h` : lecture avec sélection et repli sans indication de format, exploration de documents sectoriels/flux, résultat technique inconnu, sélection d'interprétation et détection hybride, capture déjà chargée conservée.
  - [x] Modifier `src/MediaEngine/src/Composition/Runtime.h` et `Images/Conversion/Fat12Facade.cpp` : partager l'explorateur et retirer la logique d'exploration propre à FAT12.
- [x] Conversion sectorielle vers HFE
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/FluxConversion.h`, créer `src/MediaEngine/src/Images/Conversion/HfeFile.cpp` : éligibilité raw.hfe, exploration commune, encodeur de pistes et writer HFE existants ; refuser SCP dans ce parcours sectoriel.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : intégrer le bloc.
- [x] Vérification ciblée
  - [x] Créer `build/verification-disk-explorer/CMakeLists.txt` et `main.cpp` : documents injectés et lecture par chemin, indication de format/refus/repli, sélection, source inconnue, récupération de flux rejeté, FAT12 et production HFE ; compiler le bloc.
  - [x] Supprimer `build/verification-disk-explorer/` et ses médias temporaires après utilisation.
