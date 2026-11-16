# Façades de conversion des fichiers sectoriels

- [x] Services communs
  - [x] Modifier `src/MediaEngine/src/Reconstruction/Scp/Composition.h` et `Composition.cpp` : exposer les reconstructeurs partagés ISO, Apple et Amiga sans créer un second registre.
  - [x] Modifier `src/MediaEngine/src/Images/Conversion/Service.h` et `Service.cpp` : porter le helper de sélection SCP/image des façades C#.
- [x] Conversion des familles
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaFileConversion.h`, `src/MediaEngine/src/Images/Conversion/Files.h`, `Files.cpp`, `FileSources.cpp` et `FileConstants.h` : façade commune pour IBM, MSX, BBC DFS, Acorn ADF, Amiga ADF, Amstrad DSK/EDSK, Epson IMG/IMD, UCSD IMG/TD0, DEC RX02 et Commodore D81 ; lecteurs source fidèles, conservation des conteneurs détaillés et transformations existantes.
  - [x] Modifier `Composition/Runtime.h`, `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : raccorder les façades aux services et writers partagés.
- [x] Familles complémentaires
  - [x] Modifier `MediaFileConversion.h`, `Reconstruction/Scp/Composition.h`, `Composition.cpp`, `Images/Conversion/Files.h`, `Files.cpp`, `FileSources.cpp`, `FileConstants.h`, créer `FileAppleSources.cpp` : Atari ATR/ST/MSA, Apple sectoriel/NIB/WOZ/Macintosh/Lisa, Commodore D64/D71 et COHERENT, métadonnées DiskCopy et cartes d'erreurs, règles propres au SCP.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `.filters` : intégrer les nouvelles sources au bloc complet.
- [x] Vérification ciblée
  - [x] Créer `build/verification-sector-files/CMakeLists.txt` et `main.cpp` : capacités de destination et parcours communs des familles assemblées, préservation des conteneurs détaillés, refus d'incompatibilité ; compiler le bloc.
  - [x] Supprimer `build/verification-sector-files/` et ses artefacts après utilisation.
