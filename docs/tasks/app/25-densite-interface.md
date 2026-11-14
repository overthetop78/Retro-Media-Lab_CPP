# Densité de l’interface

- [x] Enregistrer le choix de taille
  - [x] Créer `src/Common/Constants/DisplayConstants.h` et modifier `src/Infrastructure/State/PreferencesStore.h/.cpp`, `src/App/State/WorkspaceState.h/.cpp`, `src/App/Bootstrap/Application.cpp` : pourcentages autorisés, valeur initiale 100, validation et sauvegarde compatibles avec les préférences existantes.
- [x] Adapter la présentation
  - [x] Créer `src/App/Themes/RetroMediaLab/DisplayDensity.h/.cpp` et modifier `RetroMediaLabWindow.h/.cpp`, `RetroMediaLabStyle.cpp`, `ThemeConstants.h` et `src/App/CMakeLists.txt` : facteur compact du thème, adaptation à l’espace logique disponible, redimensionnement du contenu sans changement de géométrie des fenêtres, mises à jour lors des changements d’écran ou de préférence, nettoyage RAII.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/PreferencesDialog.h/.cpp` et créer `src/Translations/Catalogs/**/App/display.json` : cadre Taille de l’interface, sélecteur 50/75/100/125/150/200 %, textes traduits et mise à jour immédiate.
- [x] Vérifier et nettoyer
  - [x] Créer `build/verification-density/` : vérifier les six choix, absence de cumul, géométrie des fenêtres conservée, persistance, adaptations 2160p/150 % et 1080p/100 %, présentation claire/sombre et changements d’écran simulés.
  - [x] Supprimer `build/verification-density/` et ses artefacts après validation Debug/Release.
