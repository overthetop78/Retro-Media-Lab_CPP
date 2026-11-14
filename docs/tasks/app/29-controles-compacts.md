# ContrÃ´les compacts dans les fenÃªtres normales

- [x] Corriger les dimensions du contenu
  - [x] Modifier `src/App/Themes/RetroMediaLab/RetroMediaLabStyle.cpp`, `ThemeConstants.h` et `metrics.json` : hauteur compacte des champs et marges internes adaptÃ©es au facteur de densitÃ©.
  - [x] CrÃ©er `src/App/Themes/RetroMediaLab/ContentScrollArea.h` et `.cpp` ; modifier `DisplayDensity.cpp`, `src/Common/Constants/DisplayConstants.h` et `src/App/CMakeLists.txt` : hauteur recalculÃ©e sur le contenu rÃ©el aprÃ¨s adaptation, plafonnÃ©e pour les longues listes.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/FloppySettingsPage.cpp` : supprimer la hauteur figÃ©e prÃ©maturÃ©ment des lecteurs.
  - [x] CrÃ©er `src/App/Themes/RetroMediaLab/FlowLayout.h` et `.cpp` ; modifier `Views/GreaseweazleToolPage.cpp`, `ThemeConstants.h` et `src/App/CMakeLists.txt` : paramÃ¨tres compacts avec retour Ã  la ligne, comme le WrapPanel C#.
- [x] VÃ©rifier le rÃ©sultat
  - [x] CrÃ©er `build/verification-controls/` : banc temporaire parcourant les pages dans les fenÃªtres normales en 800Ã—600 et 1920Ã—1080 ; enregistrer les captures corrigÃ©es sous `docs/validation/app/captures/`.
  - [x] Supprimer `build/verification-controls/` et ses fichiers temporaires aprÃ¨s les vÃ©rifications.
