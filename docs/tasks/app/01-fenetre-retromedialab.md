# Étape 3 proposée : première fenêtre RetroMediaLab Light/Dark

Statut : premier socle réalisé et vérifié le 14 novembre 2026. Outils : Qt 6.12.0 MSVC x64, MSVC 14.44, CMake et Ninja.
Référence : [proposition du socle](../../design/app/socle-graphique.md).
Les actions sont réalisées par blocs. Les en-têtes publics emploient le nom complet RetroMediaLab plutôt que l'abréviation Rml proposée initialement. Le repli des traductions reprend le mécanisme C# existant.

- [x] Première fenêtre graphique
  - [x] Configuration de compilation
    - [x] Créer `CMakeLists.txt` : projet C++, versions retenues, sous-projets App, Translations et Platform, règles de copie/installation des ressources et dépendances requises.
    - [x] Créer `CMakePresets.json` : presets Windows x64 Debug/Release, répertoires de sortie sous build/, sans chemin personnel codé en dur.
    - [x] Créer `src/Translations/CMakeLists.txt` : bibliothèque séparée, API publique limitée aux traductions, exports et ressources nécessaires.
    - [x] Créer `src/Platform/CMakeLists.txt` : cible interne, sélection de l'implémentation Windows et dépendances natives privées.
    - [x] Créer `src/App/CMakeLists.txt` : exécutable graphique lié aux services nécessaires et à Qt Widgets, liste explicite des sources et déploiement Qt du dossier prêt à lancer.
    - [x] Créer `scripts/local-building/build.ps1` : configuration, compilation et préparation du dossier exécutable avec ses dépendances, arrêt sur erreur.
  - [x] Traductions de la fenêtre
    - [x] Créer `src/Translations/include/RetroMediaLab/Translations/TranslationService.h` et `src/Translations/src/TranslationService.cpp` : API de langue, résolution des textes par catalogue et clé dans Languages, accès aux invariants dans Constants et notification du changement ; sans dépendance aux contrôles App ; repli sur Constants puis marqueur de clé absente, comme en C#.
    - [x] Créer `src/Translations/Catalogs/Constants/App/window.json` : base anglaise contenant le titre invariant et les textes nécessaires à la fenêtre, au choix Light/Dark et au choix de langue, rangés par clés.
    - [x] Créer `scripts/translations/ignores/en-US.txt` et `scripts/translations/ignores/fr-FR.txt` : invariants à protéger dans les phrases, selon les textes présents.
    - [x] Créer `scripts/translations/translate.py` : génération en-US depuis la base, traduction Argos vers les langues demandées, protection des invariants et fonctionnement par boucle.
    - [x] Créer `src/Translations/Catalogs/Languages/en-US/App/window.json` et `src/Translations/Catalogs/Languages/fr-FR/App/window.json` avec le script, puis corriger les seules erreurs constatées à la relecture ; exclure les invariants isolés.
  - [x] Gestion des interfaces complètes des thèmes et adaptation Windows
    - [x] Créer `src/App/Themes/RetroMediaLab/metrics.json` : dimensions et paramètres communs repris de Tokens.xaml nécessaires au socle.
    - [x] Créer `src/App/Themes/RetroMediaLab/light.json` et `src/App/Themes/RetroMediaLab/dark.json` : rôles et couleurs des deux palettes C# relevées.
    - [x] Créer `src/App/Themes/ThemeService.h` et `src/App/Themes/ThemeService.cpp` : gestion du thème comme interface graphique complète, création et remplacement à chaud de sa présentation depuis le même état de session, notifications et coordination de la libération ; sélection Light/Dark et chargement de ses données visuelles comme fonctions complémentaires, sans imposer de navigation ou d'onglets communs à tous les thèmes.
    - [x] Créer `src/App/Themes/IThemePresentation.h` : contrat interne de création et de libération de l'interface complète d'un thème, fenêtres, navigation et organisation des contenus comprises ; rattachement aux identités et données de session sans imposer leur représentation en onglets, fenêtres ou panneaux ; aucun service métier.
    - [x] Créer `src/App/Themes/RetroMediaLab/RetroMediaLabStyle.h` et `src/App/Themes/RetroMediaLab/RetroMediaLabStyle.cpp` : style commun des contrôles présents, utilisant les valeurs du thème pour fond, texte, bordures, arrondis et états.
    - [x] Créer `src/Platform/include/RetroMediaLab/Platform/WindowAppearance.h` et `src/Platform/Windows/WindowAppearance.cpp` : contrat interne d'apparence de la fenêtre et adaptation DWM Windows, sans API Windows dans l'en-tête commun.
  - [x] Application et fenêtre
    - [x] Créer `src/App/Shared/Controls/Card.h` et `src/App/Shared/Controls/Card.cpp` : conteneur réutilisable, présentation issue du thème et disposition contrôlée par App.
    - [x] Créer `src/App/Themes/RetroMediaLab/RetroMediaLabLayout.h` et `src/App/Themes/RetroMediaLab/RetroMediaLabLayout.cpp` : composition propre à l'interface RetroMediaLab pour le premier socle, zone centrale, carte et choix Light/Dark et langue ; les futurs choix de navigation et d'organisation appartiennent à ce thème et ne sont pas imposés aux autres ; textes liés aux clés et abonnements libérés à la destruction.
    - [x] Créer `src/App/State/WorkspaceState.h` et `src/App/State/WorkspaceState.cpp` : état du socle indépendant des vues, identités des fenêtres logiques et valeurs d'interface nécessaires à la reconstruction ; ne pas créer de modèles métier fictifs.
    - [x] Créer `src/App/Themes/RetroMediaLab/RetroMediaLabWindow.h` et `src/App/Themes/RetroMediaLab/RetroMediaLabWindow.cpp` : fenêtre concrète RetroMediaLab, titre traduit/invariant résolu, accueil de RetroMediaLabLayout et services injectés ; cadre non imposé aux futurs autres thèmes.
    - [x] Créer `src/App/Windows/WindowCoordinator.h` et `src/App/Windows/WindowCoordinator.cpp` : coordination des fenêtres logiques et de la présentation du thème ; reconstruction à partir du même état, synchronisation des saisies et distinction entre remplacement visuel et fermeture utilisateur ; conservation de la présentation précédente en cas d'exception pendant la préparation.
    - [x] Créer `src/App/Bootstrap/Application.h` et `src/App/Bootstrap/Application.cpp` : ordre de démarrage langue/thème/état de session/présentation, durée de vie des services indépendante des vues et nettoyage RAII à la fermeture, sur exception et sur initialisation partielle avant affichage ; éviter de quitter lors d'un remplacement technique des fenêtres.
    - [x] Créer `src/App/main.cpp` : point d'entrée court déléguant à Application.
    - [x] Créer `src/App/Assets/app-icon.ico` : reprendre l'icône du dépôt C# pour la fenêtre et l'exécutable, avec son origine consignée.
    - [x] Modifier `src/App/CMakeLists.txt` et créer `src/Platform/Windows/App.rc` : embarquer l'icône dans l'exécutable Windows et installer la ressource nécessaire à la fenêtre.

## Contrôles après réalisation

- [x] Rangement de la distribution demandé par l'utilisateur
  - [x] Créer `src/App/AppApi.h` et `src/App/Themes/RetroMediaLab/ThemeEntry.cpp` : exports des services graphiques internes et fabrique de présentation/style du thème ; empêcher les exceptions de traverser la frontière de chargement.
  - [x] Modifier `src/App/CMakeLists.txt` : produire lib/Themes/RetroMediaLab.dll avec les fenêtres, dispositions, styles et données du thème embarquées en ressources Qt ; garder App.dll indépendant de la composition du thème.
  - [x] Modifier `src/App/Themes/ThemeService.h`, `src/App/Themes/ThemeService.cpp`, `src/App/Shared/Controls/Card.h`, `src/App/State/WorkspaceState.h`, `src/App/Windows/WindowCoordinator.h` et `src/App/Bootstrap/Application.cpp` : charger et posséder le thème, publier les seuls services nécessaires et conserver sa bibliothèque jusqu'à la destruction des styles et fenêtres.
  - [x] Modifier `CMakeLists.txt` et `scripts/local-building/build.ps1` : arrêter de distribuer les JSON externes du thème et retirer les anciennes sorties Themes/RetroMediaLab après installation de la DLL.
  - [x] Modifier `src/App/CMakeLists.txt`, `src/Translations/CMakeLists.txt`, `src/Launcher/main.cpp` et `scripts/local-building/build.ps1` : noms courts App.dll et Translations.dll directement sous lib ; retirer les anciennes sorties préfixées.
  - [x] Modifier `src/Translations/CMakeLists.txt`, `src/Translations/src/TranslationService.cpp` et `src/App/Bootstrap/Application.cpp` : catalogues sous lib/Languages, nommés constants.dll, en-US.dll, fr-FR.dll, sans préfixe ; conserver le service de traductions directement sous lib.
  - [x] Supprimer l'ancien dossier de sortie `build/<configuration>/Retro Media Lab/lib/Translations` après migration vers lib/Languages.
  - [x] Créer `src/Launcher/main.cpp` : lanceur Windows sans dépendance Qt au démarrage, configuration des chemins DLL sous lib avant de charger la bibliothèque applicative, et libération des chargements possédés.
  - [x] Créer `src/App/Bootstrap/EntryPoint.cpp` et modifier `src/App/main.cpp` : point d'entrée applicatif exporté, gestion des exceptions et entrée directe pour les autres plateformes.
  - [x] Modifier `src/App/CMakeLists.txt`, `src/Translations/CMakeLists.txt` et `CMakeLists.txt` : installation des DLL applicatives et du service directement dans lib ; DLL de catalogues sous lib/Languages avec les noms courts demandés.
  - [x] Modifier `scripts/local-building/build.ps1` : staging Qt puis classement des DLL dans lib/Qt6 avec plugins associés, lib/DirectX pour les DLL DirectX et lib/OpenGL pour les autres bibliothèques graphiques concernées ; traiter aussi les anciennes sorties produites à la racine.
  - [x] Supprimer les fichiers temporaires de staging sous `build/qt-staging/` après leur classement ; déplacer puis retirer les seuls anciens dossiers de plugins Qt produits à la racine du dossier exécutable.
  - [x] Supprimer l'ancien dossier de catalogues JSON généré sous `build/<configuration>/Retro Media Lab/Translations/Catalogs` après remplacement par les DLL de catalogues.
  - [x] Modifier `src/App/Bootstrap/Application.cpp` : recherche des plugins dans lib/Qt6/plugins avant l'initialisation graphique, sans dépendre du PATH Qt du poste.
  - [x] Créer `scripts/translations/compile_catalog.py` : regrouper les JSON sources par domaine et générer les données C++ embarquées des bibliothèques de catalogues, avec une API C minimale indépendante de Qt.
  - [x] Modifier `src/Translations/CMakeLists.txt` et `src/Translations/src/TranslationService.cpp` : produire une bibliothèque Constants et une par langue, charger et copier leurs textes puis décharger les bibliothèques, sans JSON externes dans la distribution.
  - [x] Modifier les fichiers temporaires de `build/verification/` : vérifier la lecture des catalogues compilés et les mêmes garanties de repli, de module et de durée de vie.

- [x] Corrections identifiées au premier build
  - [x] Modifier `scripts/local-building/build.ps1` et `CMakeLists.txt` : détecter le préfixe MSVC réel de /showIncludes pour préserver les dépendances d'en-têtes avec le compilateur français ; créer puis supprimer les seules sondes temporaires sous build/compiler-probe.
  - [x] Supprimer via Ninja les anciens objets de compilation sous `build/debug/` générés avant la correction de /showIncludes, puis recréer les sorties Debug avec leurs dépendances d'en-têtes complètes.
  - [x] Modifier `scripts/local-building/build.ps1` : lors de la migration des anciennes DLL à la racine, conserver la version nouvellement installée dans lib au lieu de l'écraser par une ancienne sortie.
  - [x] Modifier `src/App/Themes/RetroMediaLab/RetroMediaLabWindow.cpp` : réserver une marge pour le cadre natif lorsque la fenêtre est dimensionnée sur un petit écran ; vérifier la restauration dans les limites de l'écran.
  - [x] Modifier `src/Translations/src/TranslationService.cpp` : corriger la fermeture du namespace du chargeur initial, puis remplacer ce chargement JSON par la lecture des catalogues compilés.
  - [x] Modifier `src/Translations/include/RetroMediaLab/Translations/TranslationService.h` : exporter les méthodes publiques de la DLL sans exporter le stockage privé du pimpl.
  - [x] Modifier `src/App/Themes/ThemeService.h`, `src/App/Themes/ThemeService.cpp` et `src/App/Bootstrap/Application.cpp` : confier explicitement au service de thème la création de sa présentation complète.

Compléments identifiés avant toute création des sources :

- [x] Compléter les fichiers nécessaires au socle
  - [x] Créer `src/Platform/Unix/WindowAppearance.cpp` : adaptation portable sans DWM pour Linux/macOS, laissant le toolkit gérer le cadre natif.
  - [x] Créer `src/Translations/include/RetroMediaLab/Translations/Export.h` : export/import de la bibliothèque Windows et visibilité des autres plateformes.
  - [x] Créer `src/App/Themes/RetroMediaLab/RetroMediaLabPresentation.h` et `src/App/Themes/RetroMediaLab/RetroMediaLabPresentation.cpp` : présentation possédant les fenêtres concrètes du thème.
  - [x] Créer `build/verification/CMakeLists.txt` et `build/verification/main.cpp` : vérifications temporaires des changements de langue, du repli, des reconstructions et de la libération des vues ; sans workflow GitHub.
  - [x] Supprimer `build/verification/` et ses artefacts après les vérifications ; conserver les sources applicatives et le dossier exécutable normal.

Compilation et lancement local, vérification visuelle Light/Dark, changements répétés de thème et de langue sans redémarrage ni perte d'état, reconstruction de la présentation RetroMediaLab avec le même état, DPI et redimensionnement, textes et fermeture ; lancement depuis le dossier préparé. Vérifier le nettoyage des ressources et abonnements, y compris sur exception avant affichage et pendant la reconstruction. Ces contrôles ne demandent aucun fichier de tests ni média externe. La règle IMPORTANT s'applique également à tout test ajouté. Les contrôles des documents et onglets métier viendront avec leur portage. Si une correction nécessite des fichiers supplémentaires, ajouter ses actions après les dernières actions terminées avant de l'exécuter.

La documentation de compilation, des traductions et les références seront maintenues automatiquement. Les actions d'implémentation supplémentaires révélées par le choix précis de Qt/MSVC seront ajoutées avant exécution.


Résultat : builds Debug et Release réussis. Tests de traduction, de reconstruction et de nettoyage réussis ; sources et artefacts de tests supprimés. Voir ../../validation/app/socle-graphique.md pour la portée exacte des vérifications.
