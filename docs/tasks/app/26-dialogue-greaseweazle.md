# Dialogue GreaseWeazle

- [x] Regrouper la navigation existante
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/MainMenu.h/.cpp`, `RetroMediaLabLayout.h/.cpp` et `RetroMediaLabWindow.h/.cpp` : remplacer les sous-menus Diagnostics et Matériel par l’action GreaseWeazle après le séparateur du menu Options et ouvrir un dialogue dédié.
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/GreaseweazleDialog.h/.cpp` et modifier `src/App/Themes/RetroMediaLab/ThemeConstants.h`, `src/App/State/WorkspaceState.h` et `src/App/CMakeLists.txt` : neuf onglets correspondant aux options C#, fermeture, conservation de l’onglet et de la géométrie, adaptation au thème/langue/densité et destruction RAII.
  - [x] Modifier `src/Translations/Catalogs/Constants/App/menus.json` : ajouter le nom invariant GreaseWeazle ; réutiliser les libellés traduits existants des neuf options.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/ThemePreview.cpp` : fournir le nouveau rappel de menu sans action dans l’aperçu hors écran.
- [x] Porter les contrôles et commandes C# demandés
  - [x] Créer `src/Infrastructure/Maintenance/GwToolsConstants.h`, `GreaseweazleTools.h/.cpp` et modifier `src/Infrastructure/CMakeLists.txt` : schémas des neuf opérations, validation et construction des arguments C#, exécution asynchrone, arrêt et destruction des processus, sorties et journaux ; conserver les brouillons et résultats hors des vues.
  - [x] Modifier `src/App/State/WorkspaceState.h`, `src/App/Themes/RetroMediaLab/Views/GreaseweazleDialog.h/.cpp`, `ThemeConstants.h` et `src/App/Themes/RetroMediaLab/RetroMediaLabWindow.cpp` : formulaires des neuf onglets, lecteur sélectionné, commande affichée, exécution/arrêt, confirmation du bootloader et de l’arrêt, résultats et conservation lors des changements de thème.
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/GreaseweazleToolPage.h/.cpp` et modifier `src/App/CMakeLists.txt` : présentation commune des paramètres et des sorties, validation, exécution et confirmations des neuf opérations.
  - [x] Créer `src/Translations/Catalogs/**/App/greaseweazle-tools.json` et modifier `scripts/quality/audit_literals.py` : reprendre les traductions C# des champs, résultats et confirmations ; traduire les erreurs complémentaires et déclarer les constantes techniques.
- [x] Vérifier et nettoyer
  - [x] Créer `build/verification-greaseweazle-dialog/` : vérifier le menu, l’ouverture unique, les neuf onglets, la conservation de l’état, les langues, le thème et le pourcentage.
  - [x] Supprimer `build/verification-greaseweazle-dialog/` et ses artefacts après validation Debug/Release.
