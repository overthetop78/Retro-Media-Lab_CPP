# Signaux de bande et sons mÃ©caniques publics

- [x] Contrats et services audio publics
  - [x] CrÃ©er `src/MediaEngine/include/RetroMediaLab/MediaEngine/TapeSignal.h` en dÃ©plaÃ§ant la dÃ©claration existante, modifier `src/MediaEngine/src/Models/Sequential/TapeSignal.h` en relais, crÃ©er `TapePreparation.h` et `src/MediaEngine/src/Preparation/Tape.cpp` : lecture/Ã©criture asynchrones par les fonctions de signal existantes, paramÃ¨tres possÃ©dÃ©s, extensions dâ€™enregistrement publiques.
  - [x] CrÃ©er `src/MediaEngine/include/RetroMediaLab/MediaEngine/MechanicalSound.h`, `MechanicalSoundConstants.h`, `src/MediaEngine/src/Audio/Mechanical/Service.cpp`, `Mix.cpp` et modifier `RecordedBank.h`, `RecordedBank.cpp`, `Constants.h`, `src/MediaEngine/src/Drives/Mechanics.cpp`, `Audio.cpp` : service public de rendu/durÃ©e/prÃ©chargement, banque par dÃ©faut partagÃ©e et protection du mÃ©lange commune, sans sortie ni horloge audio possÃ©dÃ©e.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/DriveService.h` et `src/MediaEngine/src/Drives/Service.cpp` : crÃ©ation dâ€™une mÃ©canique de bande et choix explicite du relais des notifications audio hÃ´te, avec les noms techniques C#.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj`, `RetroMediaLab.vcxproj.filters`, `scripts/quality/audit_literals.py` : intÃ©grer les services et dÃ©clarer lâ€™en-tÃªte de constantes publiques sans changer les contrÃ´les.
- [x] VÃ©rification ciblÃ©e
  - [x] CrÃ©er `build/verification-public-audio/CMakeLists.txt`, `main.cpp` et sorties temporaires : signal WAV/RAW/UEF, paramÃ¨tres, annulation, donnÃ©es conservÃ©es aprÃ¨s tÃ¢che, profils et mÃ©lange, crÃ©ation Tape et notifications, rendu dâ€™un enregistrement de la banque et durÃ©e.
  - [x] Supprimer `build/verification-public-audio/` avec les sorties et exÃ©cutables temporaires aprÃ¨s utilisation.

