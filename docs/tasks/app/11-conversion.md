# Préférences de conversion

- [x] Porter les réglages C#
  - [x] Créer `src/App/Conversion/ConversionConstants.h` et `ConversionSettings.h/.cpp` : activation, modèle, cinq récents, modèles prédéfinis et aperçu déterministe repris du C#.
  - [x] Modifier `src/App/State/PreferencesStore.h/.cpp`, `WorkspaceState.h/.cpp` et `Bootstrap/Application.cpp` : conserver et sauvegarder les préférences de conversion indépendamment du thème.
- [x] Ajouter l'onglet
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/ConversionSettingsPage.h/.cpp` ; modifier `PreferencesDialog.h/.cpp`, `src/App/CMakeLists.txt` et `scripts/quality/audit_literals.py` : onglet Conversion, cartes réglages/variables, historique et aperçu, sélection de modèles, conservation de la session et constantes nommées.
  - [x] Créer `src/Translations/Catalogs/Constants/App/conversion.json` et les 29 catalogues `Languages/<langue>/App/conversion.json` : reprendre les textes existants C#, corriger les contresens repérés à la relecture et conserver les autres catalogues.
- [x] Vérifier et nettoyer
  - [x] Créer `build/verification-conversion/CMakeLists.txt` et `main.cpp` : persistance, préréglages, historique, aperçu, langues, changement de thème et conservation des réglages.
  - [x] Supprimer `build/verification-conversion/` et ses artefacts après validation.

- [x] Compacter le bloc pour la fenêtre de préférences
  - [x] Modifier `src/App/Conversion/ConversionConstants.h` et `src/App/Themes/RetroMediaLab/Views/ConversionSettingsPage.cpp` : réduire marges et espacements, limiter la hauteur de l'historique et donner aux boutons de variables une hauteur adaptée au texte pour laisser l'aperçu visible.
  - [x] Créer `build/verification-conversion-size/CMakeLists.txt` et `main.cpp` : vérifier le rendu avec la police réelle du thème à 1100 × 640 et à une échelle de 150 %, sans défilement vertical nécessaire.
  - [x] Supprimer `build/verification-conversion-size/` et ses artefacts après vérification.
