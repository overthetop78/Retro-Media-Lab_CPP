# Aperçu du thème et application explicite

- [x] Séparer les paramètres sélectionnés des paramètres actifs
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/ThemeSettingsPage.h/.cpp` et `ThemeConstants.h` : rétablir Choisir, conserver une sélection provisoire, ne modifier le thème actif et sa sauvegarde qu'au clic ; afficher le thème actif réel.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/PreferencesDialog.cpp` : transmettre l'état de session en lecture à la page d'aperçu.
  - [x] Modifier `src/Translations/Catalogs/Constants/App/themes.json` et `src/Translations/Catalogs/Languages/*/App/themes.json` : rétablir le libellé Choisir et adapter l'explication à la confirmation explicite, avec traduction Argos et relecture du seul texte modifié.
- [x] Produire une image sans modifier l'application
  - [x] Modifier `src/App/Themes/ThemeService.h/.cpp` et `src/App/Themes/RetroMediaLab/RetroMediaLabConfiguration.cpp` : préparer un service isolé, sans publication de palette ou police globale, conserver le comportement système réel lors de l'application et éviter les notifications identiques entraînant une nouvelle sauvegarde.
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/ThemePreview.h/.cpp` et modifier `src/App/CMakeLists.txt` : rendre hors écran la disposition avec un style local et une copie indépendante de l'état ; libérer vues, style, service et ressources après génération de l'image, sans fenêtre native affichée.
  - [x] Modifier `src/Translations/src/TranslationService.cpp` : retirer les abonnements expirés lors de nouveaux abonnements afin que les vues temporaires d'aperçu ne laissent pas croître la liste d'observateurs.
- [x] Vérifier et nettoyer
  - [x] Créer `build/verification-preview/CMakeLists.txt` et `main.cpp` : vérifier image différente, palette/police/service/préférences inchangés avant Choisir, application et persistance après clic, fermeture sans application et conservation des données.
  - [x] Supprimer `build/verification-preview/` et `build/preview-translations/` avec leurs artefacts après utilisation.
