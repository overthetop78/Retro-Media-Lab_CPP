# Proposition du premier socle graphique C++

Statut : premier socle implémenté le 14 novembre 2026 ; ce document conserve son découpage et les exigences des étapes suivantes.
Les choix déjà demandés sont RetroMediaLab Light/Dark, une fenêtre simple, Windows en premier, et l'émulation en dernier.

Précision pour la page Thèmes : Système/Clair/Sombre sont exclusivement des paramètres de RetroMediaLab. Système reprend les couleurs de la plateforme et leurs changements ; chaque futur thème pourra définir des paramètres différents. ThemeService transmet des paramètres génériques à son contrôleur dans la DLL, et les préférences les conservent séparément par identifiant de thème. Voir [les références techniques](../../references/tooling/couleurs-systeme-et-reglages-themes.md) et [la validation de la page](../../validation/app/page-themes.md).

## Choix technique proposé

Outils employés sous Windows : C++20, Qt 6.12.0 Widgets, MSVC 14.44 x64, CMake fourni avec Visual Studio et Ninja. Qt Creator 20.0.2 est également installé. Le build ne nécessite pas d'utiliser un éditeur précis.

Ce choix couvre les fenêtres, dialogues, contrôles et dispositions d'une application de bureau, et prévoit les autres plateformes sans remplacer le GUI. Qt reste limité à la présentation et à ses adaptations ; il ne doit pas entrer dans les contrats métier des futurs moteurs ou modules. Le SDK n'exposera pas de QWidget ni de pointeur vers la fenêtre App.

Qt Widgets est le choix du portage, pas une dépendance reprise du C#. Les palettes proviennent des valeurs WPF relevées ; la première fenêtre utilise un style commun et une carte personnalisée. Cela ne constitue pas le portage de tous les contrôles du C#.

Pour une présentation maintenable, un style commun basé sur QProxyStyle utilisera les palettes et dimensions du thème ; les cartes propres à RML utiliseront les mêmes valeurs. Éviter un grand fichier de styles reproduisant la dispersion actuelle. Un thème peut aussi définir la disposition de l'interface, et pas seulement sa palette. Les changements de thème et de langue doivent fonctionner à chaud, sans redémarrage.

## Responsabilités et organisation proposées

| Partie | Responsabilité | Accès |
|---|---|---|
| App/Bootstrap | Créer les services, fixer la langue et le thème, ouvrir et fermer la fenêtre | Composition interne à l'application |
| App/Windows | Coordination du cycle de vie des fenêtres logiques et de leurs représentations fournies par le thème | Le cadre et les fenêtres concrètes peuvent différer selon le thème |
| App/State | État durable de la session en mémoire : documents, fenêtres et onglets logiques, sélections et saisies | Indépendant de la durée de vie des vues ; pas nécessairement une sauvegarde sur disque |
| App/Shared | Contrôles communs et construction des dialogues autorisés | Propriété d'App ; aucun accès direct à la fenêtre globale depuis un module |
| App/Themes | Gestion et implémentations d'interfaces graphiques complètes : fenêtres, navigation, organisation des vues, dispositions, styles, palettes et dimensions | Aucun service métier ; contrôles communs dans Shared |
| Translations | Bibliothèque séparée regroupant service et catalogues par langue/domaine/fonctionnalité | Interface de traduction ; aucun contrôle graphique |
| Platform/Windows | Intégration Windows nécessaire au socle, notamment barre de titre Light/Dark | Isolée des fichiers communs |

Arborescence du socle (les sorties de compilation sont séparées) :

```text
CMakeLists.txt
CMakePresets.json
src/
  App/
    CMakeLists.txt
    main.cpp
    AppApi.h
    Bootstrap/
      Application.h
      Application.cpp
      EntryPoint.cpp
    Windows/
      WindowCoordinator.h
      WindowCoordinator.cpp
    State/
      WorkspaceState.h
      WorkspaceState.cpp
    Shared/
      Controls/
        Card.h
        Card.cpp
    Themes/
      ThemeService.h
      ThemeService.cpp
      IThemePresentation.h
      RetroMediaLab/
        ThemeEntry.cpp
        RetroMediaLabWindow.h
        RetroMediaLabWindow.cpp
        RetroMediaLabLayout.h
        RetroMediaLabLayout.cpp
        RetroMediaLabStyle.h
        RetroMediaLabStyle.cpp
        metrics.json
        light.json
        dark.json
    Assets/
      app-icon.ico
  Translations/
    CMakeLists.txt
    include/RetroMediaLab/Translations/TranslationService.h
    src/TranslationService.cpp
    Catalogs/
      Constants/App/window.json
      Languages/
        en-US/App/window.json
        fr-FR/App/window.json
  Platform/
    CMakeLists.txt
    include/RetroMediaLab/Platform/WindowAppearance.h
    Windows/WindowAppearance.cpp
    Windows/App.rc
    Unix/WindowAppearance.cpp
  Launcher/
    main.cpp
scripts/
  local-building/
    build.ps1
  translations/
    translate.py
    compile_catalog.py
    ignores/
      en-US.txt
      fr-FR.txt
```

Les sous-dossiers internes d'App ne justifient pas chacun une DLL. Sous Windows, l'exécutable natif retromedialab.exe charge lib/App.dll après configuration des chemins de bibliothèques. Le service de textes est lib/Translations.dll. L'interface RetroMediaLab est lib/Themes/RetroMediaLab.dll, avec ses fenêtres, dispositions, styles et JSON embarqués en ressources Qt. Les catalogues sont lib/Languages/constants.dll, en-US.dll et fr-FR.dll ; chaque DLL de langue regroupe les domaines et fichiers de cette langue. Platform reste une cible interne liée au code concerné.

Le code et les données sources de chaque composant restent rangés ensemble. Les JSON de thèmes et de textes ne sont pas distribués séparément. Les catalogues exposent une petite fonction C de lecture de données UTF-8 et n'ont pas de dépendance Qt. Les données sont copiées puis la DLL est déchargée. Le contrat interne des thèmes utilise les services C++/Qt de la même compilation : il ne constitue pas le futur SDK des modules d'émulation.

Le terme Shell est réservé à la console. Il n'y a pas de fenêtre concrète unique imposée à tous les thèmes : RetroMediaLabWindow est celle du thème RetroMediaLab ; un autre thème pourra fournir d'autres fenêtres et cadres. WindowCoordinator maintient les identités et le cycle de vie des fenêtres logiques, indépendamment de leurs représentations. RetroMediaLabLayout compose le contenu RetroMediaLab. Les noms restent proposés et les autres thèmes ne sont pas implémentés dans le premier socle.

La configuration CMake lie les dépendances nécessaires à chaque cible, avec des chemins d'en-têtes publics limités. Les futurs moteurs auront leurs propres cibles ; ils ne sont ni créés vides ni simulés maintenant.

## Démarrage et fermeture du premier socle

1. Charger la bibliothèque du thème et conserver une référence dont la durée de vie dépasse celle de QApplication et de ses styles, puis créer le contexte graphique.
2. Construire le service de traduction et charger les petits catalogues de la fenêtre.
3. Charger les ressources embarquées du thème RetroMediaLab et appliquer sa variante.
4. Créer l'état de session et WindowCoordinator, demander au thème RetroMediaLab sa fenêtre RetroMediaLabWindow et sa composition RetroMediaLabLayout, appliquer son apparence Windows, puis afficher.
5. À la fermeture, terminer la boucle et libérer les objets dans l'ordre de leurs dépendances, y compris si l'initialisation a échoué avant l'affichage.

## Changement dynamique de thème et de langue

Un thème est une interface complète de l'application. Il choisit notamment si une fonction se présente dans un onglet, une fenêtre, un panneau ou une autre organisation autorisée, et comment l'utilisateur y accède. Les fichiers de couleurs et de dimensions sont seulement des données de cette interface. Aucun système d'onglets global ni disposition de navigation unique ne doit être imposé aux thèmes par App ou par ThemeService. Light et Dark sont les variantes visuelles du même thème RetroMediaLab, pas la définition du rôle d'un thème.

Les fenêtres et onglets conservés lors d'un changement représentent les mêmes documents, fonctions et contextes ouverts, même si le nouveau thème les présente différemment. Leur identité et leur état restent dans la session ; un onglet peut ainsi devenir une fenêtre ou un panneau sans fermeture du contenu ni perte de données.

Le thème contrôle la présentation et la disposition de l'ensemble de l'interface, y compris les fenêtres concrètes et leur cadre ; les descriptions des modules restent limitées à leurs blocs autorisés. App conserve les services et l'état fonctionnel séparément de ces vues. Le rechargement au changement de thème porte sur la présentation, sans redémarrer le processus ou les services métier. Recomposer une vue ne doit pas réinitialiser une opération, perdre une saisie ou relancer un module.

Un changement Light/Dark met à jour la palette, les styles et l'apparence native des fenêtres existantes quand leur structure reste la même. Un changement de design peut remplacer toute la présentation, fenêtres comprises, à travers IThemePresentation. Avant le remplacement, synchroniser dans WorkspaceState toutes les saisies encore détenues par les contrôles. Préparer les nouvelles fenêtres à partir du même état et des mêmes services, puis basculer et libérer les anciennes vues. Leur destruction technique ne doit pas être traitée comme une fermeture utilisateur : ne pas fermer les documents, arrêter les opérations ou quitter l'application lorsque la dernière ancienne fenêtre est détruite.

Conserver intégralement les documents et données modifiées non enregistrées, fenêtres et onglets logiques ouverts, ordre et onglet actif, sélections, saisies et valeurs de réglage, positions de défilement, vues actives et opérations en cours. Les données appartiennent à la session, jamais aux objets graphiques du thème. Les dimensions et positions visuelles peuvent être adaptées à la présentation du nouveau thème tout en préservant ce qui est ouvert et son état. Les mises à jour métier arrivant pendant la préparation doivent rester prises en compte ; ne pas restaurer un ancien instantané par-dessus les données actuelles.

Si la préparation échoue par exception, libérer ses ressources partielles, garder l'état fonctionnel et laisser la présentation précédente utilisable. La première version ne crée que la présentation RetroMediaLab ; elle ne simule pas les autres thèmes.

Un changement de langue actualise tous les libellés liés à leurs clés, options et titres, sans perdre les valeurs saisies. Recalculer les tailles et dispositions nécessaires aux nouvelles longueurs de texte. Les abonnements aux changements de langue/thème sont détachés à la destruction des vues.

## Libération des ressources : règle impérative

La règle IMPORTANT de .codex/config.toml s'applique partout, y compris aux tests, aux initialisations partielles et aux fenêtres invisibles ou jamais affichées. Chaque acquisition possède immédiatement un responsable de libération : RAII, destructeur approprié et garde de portée si nécessaire. Les abonnements, fenêtres, handles, objets graphiques, ressources liées à DWM, tâches et chargements dynamiques possédés suivent cette règle. L'arrêt des tâches et leur terminaison précèdent la destruction des objets qu'elles utilisent. Ne pas dépendre uniquement d'un événement de fermeture ou du seul destructeur d'un objet dont le constructeur peut échouer.

Un appel de configuration DWM ne signifie pas automatiquement qu'un objet a été alloué ; suivre les règles de libération de l'API réellement employée. Décharger les chargements dynamiques possédés seulement après la fin de toute utilisation et la destruction de leurs objets. Ne pas décharger arbitrairement une bibliothèque utilisée par le framework.

Dans la règle utilisateur, une erreur désigne une exception de l'application. Gérer ses chemins de sortie : nettoyage RAII lors du déroulement de pile, traitement aux frontières appropriées et arrêt ordonné des services si l'exception impose de quitter. Cela vaut aussi pour les modules et les tests. Ne pas confondre cette exigence avec une terminaison forcée du processus. Le retour à une présentation précédente après exception reste possible lorsque la session et ses services sont encore valides.

Références : [RAII Microsoft](../../references/tooling/cpp-resource-lifetime.md).

PreferencesStore conserve maintenant la langue et la variante dans preferences.json, sous AppConfigLocation fourni par Qt. La sauvegarde atomique utilise QSaveFile. Ces données utilisateur sont distinctes de WorkspaceState, qui conserve la session en mémoire. Le démarrage des outils, des mises à jour et du matériel sera porté avec ces fonctionnalités.

Dans Préférences > Général, le dossier d'images par défaut et la langue sont côte à côte : destination à gauche, langue à droite, proportions 3:2. Le dossier peut être saisi ou choisi avec Parcourir ; il est sauvegardé avec les autres préférences. Les anciens fichiers sans cette clé restent compatibles. La saisie est synchronisée avant reconstruction des vues.

Le bloc des tags et modèles de noms de conversion ne doit pas être ajouté sous cette ligne. L'utilisateur a demandé de le placer ailleurs et de le présenter autrement ; ce choix reste à définir.

## Première fenêtre à réaliser à l'étape 3

- Titre Retro Media Lab et icône reprise de l'application.
- Taille initiale de référence 1360 × 820, ajustée à l'espace disponible et au DPI. Le minimum C# de 1280 × 720 est documenté, mais ne doit pas rendre cette première fenêtre inutilisable sur un espace de travail plus petit.
- Cadre de fenêtre Windows normal, redimensionnement, réduction, agrandissement et fermeture.
- Fond, cadre central et navigation reprenant le thème RetroMediaLab.
- Langue dans Préférences > Général ; variante dans Préférences > Thèmes. Aucun sélecteur dans la barre principale.
- Textes fournis par clés de traduction ; le titre invariant reste disponible dans la base, sans copie inutile dans en-US.
- Zone centrale contrôlée par App, prête à accueillir les futures vues. Ne pas afficher de faux module ni des commandes métier sans implémentation.

Le bloc suivant ajoute les sept pages encore vides, la console en lecture seule et les préférences nécessaires à la langue et à l'apparence. Les préférences métier, la progression des opérations et les dialogues d'émulation seront portés avec leurs fonctionnalités. Voir la validation de l'ossature et des préférences.

## Contrats à préserver pour la suite

Les futurs modules décriront des contrôles et des blocs autorisés : identifiant stable, clé du libellé, clés des options affichées, valeurs, dimensions et placement dans leur dialogue. Les valeurs techniques d'une option restent distinctes du texte traduit. Le module ne fournira ni design arbitraire ni accès aux menus et paramètres globaux.

Ces descriptions seront transmises par le composant autorisé, notamment Emulation pour ses modules. La représentation interne des contrôles App ne devient pas une nouvelle dépendance directe Emulation.xxx → App. Le contrat SDK précis sera défini lors de ce chantier ultérieur.

Chaque module d'émulation peut fournir et utiliser directement son propre composant Translations et ses catalogues, distribués avec le module et rangés par langue/domaine/fonctionnalité. Ses textes n'ont pas à être intégrés dans les catalogues App. Les clés sont identifiées avec leur catalogue/module pour éviter les collisions entre modules. L'utilisation de traductions propres est compatible avec les frontières de dépendances définies pour les modules ; elle ne crée pas d'accès aux composants internes d'App.

Les traductions communes restent accessibles via Emulation. La langue choisie dans l'application et ses changements à chaud doivent aussi être transmis aux traductions propres des modules, avec mise à jour des vues ouvertes. Le SDK prévoira les contrats nécessaires sans imposer les sources de l'application. Le nom et l'emplacement exacts des futurs modules restent à définir. Pour le socle, seule la bibliothèque de traduction et les textes App nécessaires sont réalisés.

Les dépendances métier autorisées restent : App ↔ MediaEngine et App ↔ Emulation ; MediaEngine ↔ MediaFileSystems ↔ MediaAnalysis ; Emulation.xxx ↔ Emulation et Chips ; Emulation accède aux médias via MediaEngine. Les échanges dans les deux sens passent par les interfaces/services concernés sans obliger les bibliothèques à former des dépendances de liaison circulaires.

## Traductions et distribution

Rangement de la distribution précisé le 14 novembre : les DLL propres à l'application (App, Translations puis les futurs moteurs) sont directement sous lib. Les dépendances Qt et leurs plugins sont sous lib/Qt6 ; les DLL DirectX sous lib/DirectX. Les modules d'émulation sont sous Modules, avec un sous-dossier par module. Les JSON de traduction restent les sources maintenues dans le dépôt ; ils sont embarqués au build sous lib/Languages, dans constants.dll puis en-US.dll, fr-FR.dll, etc., sans préfixe. Les modules pourront fournir leurs propres bibliothèques de catalogues dans leur paquet.

Dans Catalogs, Constants remplace le nom 00-Base : il contient la base anglaise, y compris les invariants. Languages regroupe les catalogues par langue, puis domaine et fonctionnalité. en-US est généré depuis Constants sans les invariants isolés ; les invariants présents dans les phrases sont conservés et protégés pour la traduction. Cette organisation s'applique aussi aux traductions propres des modules.

Les textes traduisibles affichés sont résolus dans la DLL de langue choisie, y compris en anglais via en-US. Les JSON sous Catalogs/Languages sont leurs sources. Constants fournit les invariants et le repli lorsqu'une traduction manque, comme dans le C# ; une clé absente partout donne [clé]. La base anglaise source sert à générer en-US et les autres langues.

Le script utilise Argos, déclaré installé et fonctionnel. Garder une base anglaise, copier vers en-US les textes traduisibles puis traduire depuis en-US. Les invariants dans les phrases sont protégés par le fichier commun ignores/common.txt, identique pour toutes les langues et sans exception par langue. Seuls les textes nécessaires aux vues réalisées sont inclus ; les traductions sont relues après génération. Voir la documentation de traduction.

Le script prépare les DLL Qt et leurs plugins sous lib/Qt6, les DLL DirectX sous lib/DirectX et Mesa/OpenGL sous lib/OpenGL. Les prérequis système suivront la politique utilisateur pour une future distribution publique : proposition à l'installation ou au démarrage de la distribution ZIP. Ce socle ne crée pas encore d'installateur ni de paquet de publication et ne doit pas être présenté comme une validation sur un poste vierge.

## Validation prévue après implémentation

Compiler la configuration choisie, lancer la fenêtre, comparer RetroMediaLab Light et RetroMediaLab Dark aux valeurs relevées, vérifier les états des contrôles présents, les changements répétés de thème et de langue sans redémarrage, les textes, la fermeture et le redimensionnement. Vérifier aussi la reconstruction de la présentation RetroMediaLab à partir du même état, sans créer un faux second thème. Contrôler les DPI Windows disponibles et l'absence de texte tronqué. Vérifier le lancement du dossier préparé sans dépendre d'un chemin Qt du poste de développement. Vérifier la libération des ressources après fermeture, changement de composition et exception d'initialisation, même avant affichage. Les vérifications de conservation des documents et onglets métier seront réalisées lorsque ces fonctionnalités seront portées.

Aucun test utilisant des images disque ni aucun workflow GitHub n'est nécessaire à ce socle. Ne pas annoncer une équivalence visuelle complète avant observation de la fenêtre.

## Documents associés

- [Étude du C#](../../references/retro-media-lab/socle-app-csharp.md).
- [Références Qt](../../references/tooling/qt6-socle-gui.md).
- [Liste prévue pour l'étape 3](../../tasks/app/01-fenetre-retromedialab.md).
