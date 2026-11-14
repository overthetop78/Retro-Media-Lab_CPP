# Ossature visuelle RetroMediaLab

Travail autorisé le 14 novembre 2026, par blocs. Référence C# : f770449e957f26ae5f4b7b6130ddc2c52106c7fd. Pages métier vides ; aucun moteur, matériel ou module d'émulation à porter dans cette étape.

- [x] Ossature visuelle et conservation de la session
  - [x] État indépendant des vues
    - [x] Créer `src/App/Shell/ConsoleState.h` : données de console, visibilité, hauteur, sélection et défilement, sans exécution de commande.
    - [x] Modifier `src/App/State/WorkspaceState.h` : identifiants et ordre des pages ouvertes, page active et état de console ; aucun index d'onglet imposé aux autres thèmes.
    - [x] Créer `src/App/Shared/Navigation/WorkspacePages.h` et `src/App/Shared/Navigation/WorkspacePages.cpp` : catalogue des sept pages et de leurs clés de traduction, indépendant de leur présentation en onglets.
  - [x] Contrôles partagés
    - [x] Créer `src/App/Shared/Controls/LocalizedAction.h` et `src/App/Shared/Controls/LocalizedAction.cpp` : action liée à une clé, retraduite à chaud et abonnement libéré à la destruction.
    - [x] Créer `src/App/Shared/Controls/TerminalView.h` et `src/App/Shared/Controls/TerminalView.cpp` : aperçu et sortie en lecture seule, copie locale, conservation de la sélection et du défilement, couleurs fournies par le thème.
    - [x] Créer `src/App/Shared/Controls/StatusIndicator.h` et `src/App/Shared/Controls/StatusIndicator.cpp` : texte et voyant de statut, accessibles et adaptés aux couleurs du thème.
  - [x] Interface du thème RetroMediaLab
    - [x] Créer `src/App/Themes/RetroMediaLab/Views/MainMenu.h` et `src/App/Themes/RetroMediaLab/Views/MainMenu.cpp` : menus Options, Émulation et Aide du C# ; actions métier désactivées, choix existants de langue et variante accessibles.
    - [x] Créer `src/App/Themes/RetroMediaLab/Views/WorkspaceTabs.h` et `src/App/Themes/RetroMediaLab/Views/WorkspaceTabs.cpp` : pages vides, ordre et sélection tirés de la session, titres retraduits et icônes du thème.
    - [x] Créer `src/App/Themes/RetroMediaLab/Views/StatusBar.h` et `src/App/Themes/RetroMediaLab/Views/StatusBar.cpp` : matériel non configuré, état prêt, commande de visibilité de la console ; aucun faux suivi matériel ou d'opération.
    - [x] Modifier `src/App/Themes/RetroMediaLab/RetroMediaLabLayout.h`, `src/App/Themes/RetroMediaLab/RetroMediaLabLayout.cpp` et `src/App/Themes/RetroMediaLab/RetroMediaLabWindow.cpp` : remplacer l'accueil par l'ossature, séparateur console, synchronisation des vues vers la session avant reconstruction.
    - [x] Modifier `src/App/Themes/RetroMediaLab/RetroMediaLabStyle.h` et `src/App/Themes/RetroMediaLab/RetroMediaLabStyle.cpp` : présentation des onglets, état actif, cadres et menus selon le thème RetroMediaLab.
    - [x] Modifier `src/App/Themes/ThemeService.h`, `src/App/Themes/ThemeService.cpp` et les JSON sous `src/App/Themes/RetroMediaLab/` : paramètres des onglets et couleurs de console repris des sources C#.
  - [x] Textes et compilation
    - [x] Modifier `scripts/translations/translate.py` : option de génération limitée aux fichiers demandés, pour préserver les catalogues déjà relus.
    - [x] Modifier `scripts/translations/ignores/en-US.txt` et `scripts/translations/ignores/fr-FR.txt` : protéger USB et RPM dans les phrases.
    - [x] Créer `src/Translations/Catalogs/Constants/App/menus.json`, `navigation.json`, `console.json` et `status.json` : textes anglais utiles repris du C#, prompt invariant et libellé de copie.
    - [x] Créer les fichiers correspondants sous `src/Translations/Catalogs/Languages/en-US/App/` et `fr-FR/App/` avec Argos ; relire et corriger les traductions.
    - [x] Modifier les trois fichiers `window.json` sous Constants et Languages : retirer les textes de l'accueil remplacé, conserver les libellés utiles des choix de langue et d'apparence.
    - [x] Modifier `src/App/CMakeLists.txt` : compiler les contrôles communs dans App.dll et les vues propres au thème dans RetroMediaLab.dll.
  - [x] Correction demandée après présentation
    - [x] Modifier `src/App/Themes/RetroMediaLab/Views/MainMenu.h`, `MainMenu.cpp` et `src/App/Themes/RetroMediaLab/RetroMediaLabLayout.cpp` : retirer les sélecteurs de langue et de variante de la barre du haut, sans créer un autre emplacement.
    - [x] Créer `src/App/Themes/RetroMediaLab/ThemeIcons.h` et `ThemeIcons.cpp`, modifier `src/App/CMakeLists.txt` et `Views/WorkspaceTabs.h/.cpp` : remplacer les icônes natives Qt par les symboles monochromes de navigation (lecture, écriture, conversion, visualisation, explorateur, outils, émulation), dessinés sans police Windows.
    - [x] Modifier `src/App/Themes/RetroMediaLab/RetroMediaLabStyle.h/.cpp` et `Views/MainMenu.cpp` : dimensions des menus et onglets reprises du C#, suppression de la ligne native Qt et cadre arrondi de la zone centrale.
    - [x] Modifier `src/App/Shell/ConsoleState.h` et `src/App/Themes/RetroMediaLab/Views/StatusBar.cpp` : console initialement repliée et bouton iconographique conforme à la capture, avec libellé accessible traduit.
  - [x] Vérifications et nettoyage
    - [x] Créer `build/verification-shell/CMakeLists.txt` et `build/verification-shell/main.cpp` : vérifier menus, ordre et sélection des pages, console en lecture seule, visibilité, traductions, reconstructions, exceptions et destruction ; aucun workflow GitHub.
    - [x] Supprimer `build/verification-shell/` et tous ses artefacts après utilisation.

Les builds et l'inspection visuelle complètent les vérifications. Tout travail supplémentaire nécessaire sera ajouté avant exécution. La documentation et les résultats seront mis à jour automatiquement.

Validation : [résultats et limites](../../validation/app/ossature-et-preferences.md). Les sources et artefacts de vérification temporaires ont été supprimés.
