# Page Thèmes

- [x] Reprendre la composition fournie avec le seul thème RetroMediaLab
  - [x] Couleurs système
    - [x] Modifier `src/App/Themes/ThemeService.h/.cpp`, créer `src/App/Themes/RetroMediaLab/RetroMediaLabConfiguration.h/.cpp` et modifier `ThemeEntry.cpp` : rendre le contrat commun de réglages générique et placer Système/Clair/Sombre, leurs palettes et leurs observateurs dans le seul thème RetroMediaLab.
    - [x] Modifier `src/App/State/PreferencesStore.h/.cpp` et `src/App/Bootstrap/Application.cpp` : conserver des paramètres séparés par identifiant de thème, migrer l'ancien champ appearance vers RetroMediaLab, sans imposer les paramètres RML aux autres thèmes.
    - [x] Modifier `src/App/State/PreferencesStore.cpp`, les appels d'apparence native dans `RetroMediaLabWindow.cpp` et `Views/PreferencesDialog.cpp`, ainsi que `src/Platform/include/RetroMediaLab/Platform/WindowAppearance.h` et ses implémentations Windows/Unix : sauvegarder le choix Système et laisser le système fournir les couleurs du cadre natif dans ce mode.
  - [x] Présentation de la page
    - [x] Créer `src/App/Themes/RetroMediaLab/Views/ThemeSettingsPage.h/.cpp` : liste gauche avec un thème, aperçu à droite, nom, description, choix Système/Clair/Sombre, indication du thème actif, texte de persistance et bouton Choisir désactivé pour le thème déjà actif.
    - [x] Modifier `src/App/Themes/RetroMediaLab/Views/PreferencesDialog.h/.cpp` et `src/App/CMakeLists.txt` : composer la page dédiée, synchroniser ses libellés et conserver la section lors des reconstructions.
    - [x] Modifier `src/Translations/Catalogs/Constants/App/preferences.json`, créer `themes.json` et les correspondants des 29 langues : textes de la page, génération Argos depuis en-US puis relecture, sans régénérer les anciens catalogues corrigés.
  - [x] Vérifications et nettoyage
    - [x] Créer `build/verification-themes/CMakeLists.txt` et `main.cpp` : composition, aperçu, variantes, palette système et mises à jour, persistance, conservation de session et nettoyage.
    - [x] Supprimer `build/verification-themes/` et ses artefacts après utilisation.
