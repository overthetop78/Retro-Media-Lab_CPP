# Pistes physiques des disques runtime

- [x] DonnÃ©es de flux et exÃ©cution physique
  - [x] CrÃ©er `src/MediaEngine/include/RetroMediaLab/MediaEngine/FluxTrackData.h`, `src/MediaEngine/src/Runtime/FluxConstants.h` et `FluxTrackData.cpp` : rÃ©volutions possÃ©dÃ©es immuables, sÃ©rialisation versionnÃ©e et remplacement de fenÃªtre identiques au C#.
  - [x] Modifier `RuntimeDisk.h`, `src/MediaEngine/src/Runtime/DiskState.h`, `Sectors.cpp`, `Persistence.cpp`, `Metadata.cpp`, `FormatTrack.cpp` et crÃ©er `Tracks.cpp`, `WinchesterTracks.cpp` : lecture physique, fenÃªtres FM/MFM/Adaptec, conservation des flux Ã©crits, projection sectorielle diffÃ©rÃ©e et refus de sauvegarde destructive.
  - [x] Modifier `Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` et crÃ©er `src/Translations/Catalogs/Constants/MediaEngine/runtime-tracks.json` et `Languages/<langue>/MediaEngine/runtime-tracks.json` : erreurs de donnÃ©es de flux, fenÃªtre et projection traduites.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj`, `RetroMediaLab.vcxproj.filters` : intÃ©grer les contrats et implÃ©mentations physiques.
- [x] VÃ©rification ciblÃ©e
  - [x] CrÃ©er `build/verification-runtime-tracks/CMakeLists.txt`, `main.cpp` et petites images temporaires : sÃ©rialisation, limites, fenÃªtres traversant lâ€™index et plusieurs tours, FM/MFM, publication refusÃ©e sans perte et rÃ©cupÃ©ration, paramÃ¨tres/pistes Winchester et fermeture sur exception.
  - [x] Modifier `scripts/quality/audit_literals.py` : enregistrer uniquement le nouvel en-tête `src/MediaEngine/src/Runtime/FluxConstants.h` dans la liste des fichiers de constantes nommées, sans modifier les contrôles.
  - [x] Supprimer `build/verification-runtime-tracks/` et ses images/exÃ©cutables aprÃ¨s utilisation.

