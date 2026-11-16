# Conversion directe des conteneurs de flux

- [x] Service et parité
  - [x] Créer `src/MediaEngine/src/Images/Conversion/FluxContainers.h`, `FluxContainers.cpp` et `FluxParity.cpp` : copies identiques SCP/HFE, conversion HFE/SCP sans secteurs, validation structurelle et temporelle après écriture, refus des pertes HFE v1.
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/FluxConversion.h`, modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `.filters` : exposer et intégrer le service.
- [x] Validation du parcours assemblé
  - [x] Créer `build/verification-flux-containers/CMakeLists.txt` et `main.cpp` : petites images synthétiques, aller-retour et copies identiques, refus multirévolutions et annulation ; compiler le bloc.
  - [x] Supprimer `build/verification-flux-containers/` et ses médias après utilisation.
