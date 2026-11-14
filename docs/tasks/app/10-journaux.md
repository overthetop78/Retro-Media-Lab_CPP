# Journaux — portage du C#

- [x] Reprendre les réglages et leur persistance
  - [x] Créer `src/App/Logging/LogConstants.h`, `LogSettings.h/.cpp`, modifier `src/App/State/PreferencesStore.h/.cpp`, `WorkspaceState.h/.cpp` et `Bootstrap/Application.cpp` : reprendre les 14 actions C#, leurs valeurs par défaut et le repli global ; sauvegarder les modifications indépendamment du thème.
- [x] Reprendre l'écriture des journaux
  - [x] Créer `src/App/Logging/ConsoleLogSession.h/.cpp` : fichiers par action, activation, limite UTF-8 en KiB, zéro sans limite, conservation des dernières lignes ou archives datées, réglages consultés à chaque écriture et ressources fermées à chaque sortie.
  - [x] Créer `src/App/Logging/DiagnosticLog.h/.cpp` et `LogPaths.h/.cpp`, `src/Platform/include/RetroMediaLab/Platform/DataDirectory.h`, ses implémentations Windows/Unix ; modifier `src/Platform/CMakeLists.txt` et `src/App/Bootstrap/EntryPoint.cpp` : reprendre le dossier C# (Roaming ou Data en mode portable), les journaux quotidiens erreurs/avertissements/informations et raccorder les exceptions applicatives actuelles.
- [x] Ajouter la page Journaux
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/LogSettingsPage.h/.cpp`, modifier `PreferencesDialog.h/.cpp`, `ThemeConstants.h` et `ThemeIcons.cpp`/`icons.json` si nécessaire : 14 cartes, activation, taille maximale validée, archives, défilement, chemin et ouverture du dossier, libellés et couleurs à chaud.
  - [x] Modifier `src/App/CMakeLists.txt` et `scripts/quality/audit_literals.py` : compiler les fichiers et référencer les constantes propres aux journaux dans l'audit.
  - [x] Modifier `src/App/Themes/RetroMediaLab/RetroMediaLabStyle.cpp` et `Views/ThemePreview.cpp` : conserver le fond normal des champs numériques (le cadre enfoncé d'un champ n'est pas un bouton pressé) et retirer l'avertissement de masquage de QWidget relevé au build.
  - [x] Créer `src/Translations/Catalogs/Constants/App/logs.json` et ses correspondants des 29 langues en reprenant les ressources C# ; modifier `errors.json`, `src/Common/ErrorKeys.h` et leurs traductions pour les nouveaux diagnostics, sans remplacer les textes déjà relus.
- [x] Vérifier et nettoyer
  - [x] Créer `build/verification-logs/CMakeLists.txt` et `main.cpp` : réglages, sauvegarde/rechargement, validation, langues, session/thème, UTF-8, limites, archives, collisions, diagnostics quotidiens, chemins portables et erreurs d'accès dans des dossiers temporaires.
  - [x] Supprimer `build/verification-logs/` et les ressources temporaires de génération de traduction après utilisation.
