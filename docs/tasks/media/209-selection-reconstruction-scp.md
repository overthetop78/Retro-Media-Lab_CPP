# Sélection des reconstructeurs SCP

- [x] Registre et lecteur
  - [x] Créer `src/MediaEngine/src/Reconstruction/Scp/Candidates.h` et `Candidates.cpp` : candidats possédés, sélection explicite, ordre des familles et candidats automatiques.
  - [x] Créer `src/MediaEngine/src/Reconstruction/Scp/Reader.h` et `Reader.cpp` : lecture chemin/flux, priorité au premier système de fichiers reconnu, premier décodage de repli et conservation des causes originales.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : intégrer les nouvelles sources.
  - [x] Créer `src/MediaEngine/src/Reconstruction/Scp/Constants.h`, `Composition.h` et `Composition.cpp`, modifier `Composition/Runtime.h`, `Composition/Runtime.cpp` et `scripts/quality/audit_literals.py` : raccordement partagé des reconstructeurs ISO, Amiga, Atari, Commodore, Apple, RX02 et définitions ; inscriptions et ordres C# ; conservation du lecteur de flux brut.
- [x] Vérification ciblée
  - [x] Créer `build/verification-scp-selection/CMakeLists.txt` et `main.cpp` : ordre, sélection explicite, repli, erreurs, annulation et priorité à la reconnaissance ; compiler.
  - [x] Supprimer `build/verification-scp-selection/` et ses artefacts après vérification.
