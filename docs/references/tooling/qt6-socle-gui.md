# Qt 6 : références pour la proposition de socle

Références initiales consultées le 13 novembre 2026. Qt 6.12.0 MSVC x64 a ensuite été installé et employé pour le socle le 14 novembre 2026 ; les pages génériques de Qt peuvent évoluer.

- [Qt Widgets](https://doc.qt.io/qt-6/qtwidgets-index.html) : contrôles de bureau, fenêtres, dialogues, dispositions et séparation modèle/vue. Adapté au shell et aux formulaires de RML.
- [Plateformes](https://doc.qt.io/qt-6/supported-platforms.html) : Windows, Linux et macOS figurent parmi les plateformes de bureau. Vérifier la combinaison version Qt/OS/compilateur retenue avant installation.
- [CMake](https://doc.qt.io/qt-6/cmake-get-started.html) : configuration de cibles Qt par find_package, qt_add_executable et target_link_libraries ; l'exemple officiel montre une application Widgets.
- [QProxyStyle](https://doc.qt.io/qt-6/qproxystyle.html) : personnalisation du dessin, des métriques et de la palette par un style commun, pour reprendre l'apparence des contrôles du thème RetroMediaLab.
- [Style Sheets](https://doc.qt.io/qt-6/stylesheet.html) : outil de personnalisation textuelle. La documentation consultée oriente le rendu de production vers QStyle et déconseille de remplacer les feuilles au runtime pour changer de thème. La proposition utilise donc un style et des palettes communs.
- [Déploiement Windows](https://doc.qt.io/qt-6/windows-deployment.html) : windeployqt rassemble les dépendances Qt et les plugins nécessaires ; les autres bibliothèques externes restent à traiter séparément. Le runtime Microsoft suit sa procédure de redistribution officielle.
- [Licences](https://doc.qt.io/qt-6/licensing.html) : plusieurs régimes et variations par composant. Au choix de la distribution, relever les termes des composants réellement utilisés et préparer les notices correspondantes ; ne pas supposer une licence identique pour tous les modules Qt.

Ces références justifient les capacités du framework. Elles ne prouvent pas le rendu effectif de RML ni les performances d'une future émulation.

## Interfaces complètes : vérification du 14 novembre 2026

- [QWidget](https://doc.qt.io/qt-6/qwidget.html) permet de construire des contrôles personnalisés et de définir leur dessin et leurs interactions. L'interface n'est pas limitée à l'apparence des contrôles fournis.
- [QMdiArea](https://doc.qt.io/qt-6/qmdiarea.html) fournit des sous-fenêtres dans une zone commune et propose aussi un mode à onglets. C'est une possibilité de composition pour un environnement de type Windows 3.x, pas une obligation pour tous les thèmes.
- [Window Flags](https://doc.qt.io/qt-6/qtwidgets-widgets-windowflags-example.html) montre la personnalisation des fenêtres de premier niveau, notamment sans cadre natif. Un cadre personnalisé doit aussi réaliser les comportements nécessaires, tels que déplacement et redimensionnement.

Conclusion de conception : des interfaces inspirées de Windows 3.x, Windows 9x/XP, AmigaOS ou Atari TOS sont réalisables par composition et dessin personnalisés. Qt ne fournit pas automatiquement ces environnements ; leurs cadres, contrôles, navigation, icônes et interactions restent à implémenter. Les effets graphiques plus élaborés devront être évalués selon le thème concerné.

## Inventaire avant installation : 14 novembre 2026

- Visual Studio Community 2026 détecté dans C:/Program Files/Microsoft Visual Studio/18/Community.
- Outils MSVC 14.44.35207 et 14.51.36231 présents ; cl.exe 14.44 vérifié.
- CMake 4.3.1-msvc1 et Ninja fournis dans l'installation Visual Studio, hors PATH courant.
- Aucun Qt6Config.cmake, qmake.exe, qtpaths.exe ou windeployqt.exe trouvé dans cette installation Visual Studio. Aucun kit Qt repéré dans les emplacements usuels examinés ni dans les entrées d'installation Qt du registre. Cela ne prouve pas l'absence d'une copie dans un autre chemin non déclaré.
- [Qt VS Tools : ajouter une version Qt](https://doc.qt.io/qtvstools/qtvstools-how-to-add-qt-versions.html) précise qu'il faut fournir une version Qt construite pour MSVC ; l'intégration Visual Studio ne remplace pas ce kit.
- [Installation officielle Qt](https://doc.qt.io/qt-6/qt-online-installation.html) : sélectionner le kit de bureau Windows MSVC x64 par installation personnalisée.

Conformément aux règles du projet, le chemin d'un kit existant ou son installation a été demandé à l'utilisateur. Ne pas contourner ce prérequis par une autre bibliothèque graphique ni prétendre avoir compilé le socle.

Installation ensuite terminée : kit C:/Qt/6.12.0/msvc2022_64 et Qt Creator 20.0.2 vérifiés. Les builds Debug et Release du socle ont réussi. Voir [validation](../../validation/app/socle-graphique.md) et [compilation](../../development/build.md).
