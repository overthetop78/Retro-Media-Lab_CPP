# Conversions SCP de prÃ©paration runtime

- [x] Reconstruction et conversion des captures
  - [x] CrÃ©er `src/MediaEngine/src/Reconstruction/Iso/Automatic.h`, `Automatic.cpp`, `RuntimeFamilies.cpp`, `RuntimeConstants.h` : porter la sÃ©lection automatique Epson/ADFS/BBC/IBM/Atari et les constructeurs nÃ©cessaires en rÃ©utilisant les mesures, gÃ©omÃ©tries, sondes FAT et constructeurs sectoriels existants.
  - [x] CrÃ©er `src/MediaEngine/src/Reconstruction/AmigaReader.h`, `AmigaReader.cpp` : sÃ©lection des champs dÃ©codÃ©s Amiga par adresse physique, candidat dâ€™intÃ©gritÃ© prioritaire, infÃ©rence DD/HD et conservation des positions indisponibles.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaPreparation.h` et crÃ©er `src/MediaEngine/src/Preparation/ScpConversion.cpp` : convertir vers ATR/ST/ADF avec validation de famille, lecteurs et Ã©crivains existants, source conservÃ©e et annulation transmise.
  - [x] Modifier `Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` et crÃ©er les catalogues `src/Translations/Catalogs/Constants/MediaEngine/scp-preparation.json`, `Languages/<langue>/MediaEngine/scp-preparation.json` : diagnostics de famille et de cible traduits.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj`, `RetroMediaLab.vcxproj.filters`, `scripts/quality/audit_literals.py` : intÃ©grer reconstruction/conversion et enregistrer les constantes nommÃ©es.
- [x] VÃ©rification ciblÃ©e
  - [x] CrÃ©er `build/verification-scp-preparation/CMakeLists.txt`, `main.cpp` et captures/images temporaires : branches de sÃ©lection, infÃ©rence Amiga DD/HD, conversion complÃ¨te ATR/ST/ADF, familles/cibles refusÃ©es, conservation de source et annulation.
  - [x] Supprimer `build/verification-scp-preparation/` et ses captures, images et exÃ©cutables temporaires aprÃ¨s utilisation.
