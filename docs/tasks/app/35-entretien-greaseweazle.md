# Entretien Greaseweazle

- [x] Déplacer les commandes de l’onglet Outils vers le dialogue Greaseweazle
  - [x] Modifier `src/Infrastructure/Maintenance/GwToolsConstants.h` et `GreaseweazleTools.cpp` pour reprendre les options et valeurs du C# et vérifier la cible.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/GreaseweazleDialog.cpp`, `GreaseweazleToolPage.h`, `.cpp` et `ThemeConstants.h` pour regrouper les deux opérations et confirmer l’effacement du lecteur affiché.
  - [x] Reporter les chaînes Tools.* depuis le C# dans `src/Translations/Catalogs/Constants/App/greaseweazle-tools.json` et le fichier correspondant de chaque langue.
  - [x] Modifier `src/App/State/WorkspaceState.h`, `src/App/Shared/Navigation/WorkspacePages.cpp` et `src/Common/Constants/AppConstants.h` pour retirer l’onglet Outils.
  - [x] Créer puis supprimer `build/verification-gw-maintenance/` pour vérifier la construction des commandes et la confirmation sans matériel.
