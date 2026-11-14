# Couleurs système et réglages propres aux thèmes

Sources officielles consultées le 14 novembre 2026 :

- [QStyleHints.colorScheme](https://doc.qt.io/qt-6/qstylehints.html#colorScheme-prop) : la notification du changement de schéma peut précéder la mise à jour de la palette. Relire les couleurs après traitement des événements.
- [QApplication, implémentation Qt 6.12](https://github.com/qt/qtbase/blob/6.12/src/widgets/kernel/qapplication.cpp) : `basePalette` prend en compte la palette de la plateforme ; `setPalette` applique la palette avec son masque de résolution. Une palette sans couleurs explicites retire les remplacements applicatifs.
- [QGuiApplication, implémentation Qt 6.12](https://github.com/qt/qtbase/blob/6.12/src/gui/kernel/qguiapplication.cpp) : `setPalette(QPalette())` rétablit une palette résolue à partir de la plateforme sans conserver le masque des remplacements explicites.
- [DWMWINDOWATTRIBUTE](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute) : `DWMWA_COLOR_DEFAULT` rend au système le choix des couleurs de titre et de texte.

Dans RetroMediaLabConfiguration, Système retire les remplacements de palette, reprend Window, Base, Button, WindowText, Text, Mid, Accent et Highlight fournis par Qt, et observe ApplicationPaletteChange, ThemeChange et colorSchemeChanged. Les notifications sont regroupées dans un rappel différé lié au QObject ; sa destruction supprime filtre, connexions et rappels. Clair et Sombre utilisent les palettes embarquées du thème.

Ces trois choix appartiennent au seul thème RetroMediaLab. ThemeService transmet un objet de paramètres à la DLL chargée ; son contrôleur valide et applique ses propres paramètres. Aucun enum commun n'impose Système/Clair/Sombre aux thèmes futurs. PreferencesStore conserve les objets sous themeSettings/<identifiant du thème> ; l'ancien champ appearance de la version 1 est migré uniquement vers RetroMediaLab. La version 2 conserve les objets des autres thèmes sans interpréter leur contenu.

L'API interne de thème passe en version 2 pour les points d'entrée de configuration et l'identifiant. Le contrôleur est détruit avant déchargement de la DLL. Les erreurs sont copiées dans un tampon à la frontière DLL ; aucune exception du module n'y reste vivante.
