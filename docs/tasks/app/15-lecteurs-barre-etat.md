# Lecteurs configurés dans la barre d'état

- [x] Afficher et sélectionner les lecteurs
  - [x] Modifier `src/App/State/WorkspaceState.h` : conserver l'identifiant du lecteur sélectionné dans la session, indépendamment du thème.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/StatusBar.h/.cpp`, `RetroMediaLabLayout.cpp` et `ThemeConstants.h` : remplacer le texte fixe par les lecteurs configurés, sélecteur si plusieurs, disponibilité, actualisation matériel/langue/thème, retour au premier lecteur après suppression.
  - [x] Modifier les catalogues `src/Translations/Catalogs/Constants/App/hardware.json` et `Languages/*/App/hardware.json` : reprendre les libellés C# DriveChoice et Disconnected dans les 29 langues.
- [x] Vérifier
  - [x] Créer `build/verification-status-readers/CMakeLists.txt` et `main.cpp` : absence/présence de lecteurs, sélection multiple, mises à jour, langues, disponibilité et reconstruction du thème.
  - [x] Supprimer `build/verification-status-readers/`, `build/status-stage/` et leurs artefacts après validation.

