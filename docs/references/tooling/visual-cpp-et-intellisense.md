# Visual C++, IntelliSense et langue du système

Sources officielles consultées le 14 novembre 2026 :

- [Configuration IntelliSense](https://code.visualstudio.com/docs/cpp/configure-intellisense) et [référence des propriétés C++](https://code.visualstudio.com/docs/cpp/customize-cpp-settings/) : compile_commands.json fournit la configuration réelle d'une unité de traduction ; compilerPath permet de retrouver les en-têtes système du compilateur. Les chemins Qt restent explicites pour les fichiers sans entrée correspondante.
- [Projet Visual C++ Makefile](https://learn.microsoft.com/en-us/cpp/build/reference/creating-a-makefile-project?view=msvc-170) et [propriétés NMake](https://learn.microsoft.com/en-us/cpp/build/reference/nmake-property-page?view=msvc-170) : un projet .vcxproj peut fournir les commandes de compilation existantes, les fichiers affichés, les chemins d'inclusion et les options IntelliSense. Cela conserve la définition de compilation CMake.
- [QLocale::uiLanguages](https://doc.qt.io/qt-6/qlocale.html#uiLanguages) : liste ordonnée des langues d'interface, pouvant contenir des balises de script. Le démarrage utilise la première langue d'interface du système, avec name() si la liste est vide. Le catalogue RML résout langue, script et territoire, puis la langue disponible ; une langue non prise en charge revient à en-US.

La solution est à la racine. update-solution.py synchronise la liste et les filtres à partir des fichiers présents sous src et scripts ; build.ps1 l'exécute avant la configuration CMake. Les propriétés NMake appellent ce même script en Debug/Release x64. Le kit peut être remplacé par QtRoot ou RETROMEDIALAB_QT_DIR.

La configuration VS Code lit build/debug/compile_commands.json ; aucun diagnostic n'est désactivé. Les chemins de secours correspondent au kit Qt/MSVC installé sur ce poste. Les fichiers de projet et la base de compilation sont vérifiés ; la disparition visuelle des soulignements dans une session déjà ouverte de VS Code nécessite que l'extension recharge sa configuration et n'est pas déduite d'une simple compilation.
