# Application immédiate du thème

Étape historique remplacée par [la confirmation explicite](09-apercu-et-confirmation-theme.md), suivant la correction utilisateur. Choisir a été rétabli ; les choix provisoires ne modifient plus l'application.

- [x] Supprimer le bouton de confirmation inutile
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/ThemeSettingsPage.h/.cpp` : supprimer le bouton Choisir, son membre et ses libellés ; conserver les changements et leur sauvegarde immédiats.
  - [x] Modifier `src/App/Themes/RetroMediaLab/ThemeConstants.h` : supprimer les identifiants et la dimension propres au bouton retiré.
  - [x] Modifier `src/Translations/Catalogs/Constants/App/themes.json` et `src/Translations/Catalogs/Languages/*/App/themes.json` : supprimer la clé inutilisée Themes.Choose dans la base et les 29 langues.

Validation : compilation Debug et audit des constantes réussis ; aucune référence au bouton ou à sa clé ne subsiste dans les sources et catalogues.
