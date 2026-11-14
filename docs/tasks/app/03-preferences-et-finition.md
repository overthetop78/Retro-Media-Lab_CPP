# Préférences et finition de l'ossature

Suite du bloc graphique autorisé. Les captures C# restent la référence. Les pages métier et l'émulation ne sont pas portées dans ce bloc.

- [x] Préférences et finition
  - [x] État et préférences
    - [x] Créer `src/App/State/PreferencesStore.h/.cpp` : charger et sauvegarder atomiquement langue et variante dans un fichier de préférences utilisateur, valider avant mutation.
    - [x] Modifier `src/App/State/WorkspaceState.h` : conserver ouverture, section active et géométrie des préférences indépendamment du thème.
    - [x] Modifier `src/App/Bootstrap/Application.cpp` : restaurer les préférences, sauvegarder les changements, conserver l'ordre de destruction.
  - [x] Vues du thème
    - [x] Créer `src/App/Themes/RetroMediaLab/Views/PreferencesDialog.h/.cpp` : fenêtre Général/Langue et Thèmes/RetroMediaLab/Clair/Sombre, traduction et apparence immédiates, état conservé à la reconstruction, sans fausses fonctionnalités métier.
    - [x] Modifier `src/App/Themes/RetroMediaLab/Views/MainMenu.h/.cpp`, `RetroMediaLabLayout.h/.cpp` et `RetroMediaLabWindow.h/.cpp` : activer Préférences, posséder et détruire explicitement sa fenêtre, préserver son état avant reconstruction.
    - [x] Modifier `src/App/Themes/RetroMediaLab/ThemeIcons.cpp` : ajouter les symboles de préférences et de copie.
    - [x] Modifier `src/App/Shared/Controls/TerminalView.h/.cpp` : copie iconographique fournie par le thème et conservation complète des sélections/défilements de commande et sortie.
    - [x] Modifier `src/App/Shell/ConsoleState.h` et les fichiers de vues du thème ajoutés au bloc 02 : ranger et formater le code, éviter les affectations de l'état pendant la construction des vues.
    - [x] Modifier `src/App/Themes/RetroMediaLab/light.json`, `dark.json`, `RetroMediaLabStyle.cpp` et `src/App/Themes/ThemeService.cpp` : corriger le rendu avec les couleurs observables des captures, les libellés et symboles actifs, et le cadre central.
    - [x] Modifier `src/App/CMakeLists.txt` : intégrer les nouveaux fichiers dans leur bibliothèque.
  - [x] Adaptation Windows
    - [x] Modifier `src/Platform/include/RetroMediaLab/Platform/WindowAppearance.h`, `src/Platform/Windows/WindowAppearance.cpp`, `src/Platform/Unix/WindowAppearance.cpp`, les deux fenêtres du thème, les palettes JSON et `src/App/Themes/ThemeService.cpp` : appliquer les couleurs de titre fournies par le thème aux attributs DWM, sans acquisition de ressources, et conserver l'adaptation Unix.
  - [x] Traductions
    - [x] Modifier `scripts/translations/translate.py` : générer en-US depuis Constants avant de l'utiliser comme source effective des autres langues ; rejeter les fichiers ciblés inexistants avant écriture.
    - [x] Créer `src/Translations/Catalogs/Constants/App/preferences.json` et les fichiers correspondants sous `Languages/en-US/App/` et `Languages/fr-FR/App/` : libellés de préférences, génération Argos puis relecture.
  - [x] Vérification et nettoyage
    - [x] Modifier `build/verification-shell/main.cpp` et `CMakeLists.txt` : ajouter préférences, chargement/sauvegarde, traduction à chaud et destruction après erreur aux vérifications existantes ; fichiers temporaires isolés dans le dossier de test.
    - [x] Supprimer `build/verification-shell/` avec ses sources, sorties et préférences de test après vérification.

Validation : [résultats et limites](../../validation/app/ossature-et-preferences.md). Les sources et artefacts de vérification temporaires ont été supprimés.
