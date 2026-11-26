# Dossiers des sélecteurs dans les préférences

- [x] Regrouper la destination de Lecture et les dossiers de navigation
  - [x] Créer `src/Common/Constants/FileDialogPreferences.h` et modifier `src/Infrastructure/State/PreferencesStore.h` et `.cpp` pour lire et écrire les chemins dans preferences.json, et importer une fois les valeurs QSettings existantes.
  - [x] Modifier `src/App/State/WorkspaceState.h` et `.cpp`, et `src/App/Bootstrap/Application.cpp` pour restaurer et sauvegarder les dossiers par la même voie que les autres préférences.
  - [x] Modifier `src/App/Shared/Controls/RememberedFileDialog.h`, `RememberedFileDialogConstants.h` et les appels dans `src/App/Themes/RetroMediaLab/Views/{CapturePage,ConversionPage,ExplorerPage,VisualizationPage,MigrationDialog,PreferencesDialog,HostToolsSettingsWidget,KryoFluxSettingsWidget}.cpp` pour utiliser les dossiers du workspace.
  - [x] Modifier `scripts/quality/audit_literals.py` pour référencer les constantes du nouveau fichier et utiliser puis supprimer `build/verification-folder-preferences/` pour vérifier la migration et la persistance.
