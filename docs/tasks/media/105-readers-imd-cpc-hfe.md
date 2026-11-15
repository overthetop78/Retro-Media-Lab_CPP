# Readers communs ImageDisk, CPC DSK et HFE

- [x] Contrats de lecture
  - [x] CrÃ©er `src/MediaEngine/src/Formats/ImageDisk/Adapter.h` et `Adapter.cpp`, modifier `Constants.h` : capacitÃ©s IMD/Epson, prÃ©fixe Epson, sonde par signature et document conservant les mÃ©tadonnÃ©es ImageDisk.
  - [x] CrÃ©er `src/MediaEngine/src/Formats/CpcDsk/Adapter.h` et `Adapter.cpp` : capacitÃ©s DSK/EDSK, signatures standard/Ã©tendue et document sectoriel neutre utilisant le backend existant.
  - [x] CrÃ©er `src/MediaEngine/src/Formats/Hfe/Adapter.h` et `Adapter.cpp`, modifier `Constants.h` : capacitÃ©s HFE v1/v3, sonde et document flux conservant pistes, protections et mÃ©tadonnÃ©es.
- [x] IntÃ©gration
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler les trois adaptateurs ; rÃ©gÃ©nÃ©rer les projets Visual C++ existants.
- [x] VÃ©rification du raccordement
  - [x] CrÃ©er `build/verification-container-readers/CMakeLists.txt` et `main.cpp` : vÃ©rifier uniquement signatures, capacitÃ©s et conservation des reprÃ©sentations/mÃ©tadonnÃ©es Ã  travers ces nouveaux adaptateurs.
  - [x] Supprimer `build/verification-container-readers/` et ses artefacts aprÃ¨s consignation du rÃ©sultat.
