# PrÃƒÂ©paration publique des disques et catalogue

- [x] CrÃƒÂ©ation de mÃƒÂ©dias depuis les services existants
  - [x] CrÃƒÂ©er `src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaPreparation.h` et `src/MediaEngine/src/Preparation/Disks.cpp` : crÃƒÂ©ation asynchrone des disquettes vierges et des HDD Acorn DAT/DSC ou MFM EMU, prÃƒÂ©paration ADFS optionnelle, annulation et prÃƒÂ©allocation transmises aux ÃƒÂ©crivains existants.
  - [x] CrÃƒÂ©er `src/MediaEngine/src/Preparation/CatalogConstants.h` et `Catalog.cpp` : reprendre toutes les constantes DiskImageFileExtensions du C# avec leurs noms invariants et exposer les tailles optiques depuis les constantes existantes.
  - [x] CrÃ©er `src/MediaEngine/src/Composition/Runtime.h`, `Runtime.cpp` et modifier `src/MediaEngine/src/Runtime/DiskState.h`, `Disk.cpp` : partager la composition existante entre prÃ©paration et runtime sans dÃ©pendance de la prÃ©paration aux Ã©tats internes des disques.
  - [x] Modifier les diagnostics `Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` et créer `src/Translations/Catalogs/Constants/MediaEngine/preparation.json`, `Languages/<langue>/MediaEngine/preparation.json` : diagnostic de préparation traduit, distinct des erreurs de lecture et du conteneur EMU.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj`, `RetroMediaLab.vcxproj.filters` et `scripts/quality/audit_literals.py` : intÃƒÂ©grer les fonctions publiques et enregistrer uniquement lÃ¢â‚¬â„¢en-tÃƒÂªte du catalogue de constantes dans lÃ¢â‚¬â„¢audit existant.
- [x] VÃƒÂ©rification ciblÃƒÂ©e
  - [x] CrÃƒÂ©er `build/verification-media-preparation/CMakeLists.txt`, `main.cpp` et images temporaires : crÃƒÂ©ation et lecture IBM, DAT/DSC et EMU, ADFS, refus dÃ¢â‚¬â„¢ÃƒÂ©craser les HDD existants, profils non pris en charge, annulation et conservation des sources.
  - [x] Supprimer `build/verification-media-preparation/` avec ses fichiers et exÃƒÂ©cutables temporaires aprÃƒÂ¨s utilisation.

