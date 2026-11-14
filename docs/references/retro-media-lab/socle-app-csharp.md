# Démarrage, fenêtre principale et thème Base du C#

Consultation : 13 novembre 2026, via GitHub en lecture seule.
Révision étudiée : `f770449e957f26ae5f4b7b6130ddc2c52106c7fd`.
Cette étude porte sur le socle graphique, pas sur la validation des moteurs métier.

## Démarrage constaté

`App.xaml` charge les paramètres visuels communs et les styles Base.
`App.OnStartup` traite la migration des anciens dossiers, les arguments de mise à jour et les commandes des modules avant le démarrage normal. Il branche la gestion des erreurs, prépare les dossiers, charge les réglages et recherche les installations des outils hôtes. Il résout ensuite la langue et les cultures, configure le thème, démarre la surveillance GameInput, construit MainWindow et l'affiche. Un garde des raccourcis clavier est également installé.

À la fermeture, App libère les fenêtres, termine les services concernés et détache les événements. Le contrôleur de cycle de vie de MainWindow gère également l'annulation des opérations, la sauvegarde des réglages et l'arrêt de l'émulation avec une échéance.

Conséquence pour le premier socle : reprendre l'ordre langue → thème → fenêtre, mais isoler la composition des services de la construction visuelle. Les migrations, outils hôtes, opérations et émulations seront réintroduits pendant leurs portages respectifs.

## Fenêtre constatée

`MainWindow.xaml` définit une fenêtre centrée de 1360 × 820, avec un minimum de 1280 × 720, en unités logiques WPF. Son titre utilise la clé App.Title.

Organisation verticale :
1. Menu de l'application.
2. Onglets Lire, Écrire, Convertir, Visualiser, Explorer, Outils, Émulation.
3. Séparateur redimensionnable.
4. Terminal, hauteur initiale 190.
5. Barre d'état.

Les libellés sont obtenus par clés de traduction. Les contrôles métiers sont déjà des composants distincts.

Le constructeur MainWindow crée toutefois directement MediaEngineComposition ainsi que de nombreux contrôleurs et services : opérations, matériel, profils, explorateur, visualisation, terminal, etc. Le contrôleur de cycle de vie charge les capacités, les profils et le matériel. Porter ce constructeur tel quel ferait dépendre la première fenêtre de fonctions qui ne sont pas encore portées.

Le premier résultat demandé en C++ est une fenêtre simple ; cette organisation complète est une référence pour les ajouts ultérieurs, pas une liste de fonctions à créer immédiatement.

## Thème Base constaté

Les palettes Light et Dark contiennent les mêmes rôles :

| Rôle C# | Light | Dark |
|---|---|---|
| `AccentBrush` | `#4D76E8` | `#4D76E8` |
| `WindowBrush` | `#F1F2F4` | `#17191F` |
| `CardBrush` | `#FFFFFF` | `#23262E` |
| `ControlBrush` | `#FFFFFF` | `#2B2F38` |
| `TextBrush` | `#20242C` | `#F2F3F5` |
| `BorderBrush` | `#E1E4EA` | `#454B57` |
| `HoverBrush` | `#EEF2FF` | `#343A46` |
| `SelectedBrush` | `#DDE6FF` | `#3D4963` |
| `ExplorerSelectionBrush` | `#E7E9EC` | `#34373D` |
| `MutedTextBrush` | `#4D5562` | `#CAD0DA` |
| `StatusBrush` | `#FFFFFF` | `#20232A` |
| `Theme.ErrorBrush` | `#B3261E` | `#FF9B96` |
| `Theme.SuccessBrush` | `#247A47` | `#76D79C` |
| `Theme.WarningBrush` | `#986600` | `#FFD17C` |
| `Theme.DangerSurface` | `#FBE6E4` | `#48282B` |
| `Theme.SuccessSurface` | `#E4F1E9` | `#233E30` |
| `Theme.PlotBackground` | `#090C10` | `#090C10` |
| `Theme.PlotText` | `#D9E3EC` | `#D9E3EC` |
| `SyntheticNameBrush` | `#B86E00` | `#FFD17C` |

Paramètres communs relevés dans Tokens.xaml :
- Police Segoe UI ; taille 14 unités logiques.
- Hauteur de contrôle 36 ; hauteur de ligne de tableau 30.
- Espacement interne des boutons 16,8 ; champs 10,7 ; listes 10,6.
- Espacement interne des onglets 22,11 ; cartes 18.
- Bordure de carte 1 ; arrondi des contrôles 4 ; cartes 8.
- Navigation en haut ; terminal en bas ; marge de l'espace central 16,10,16,0.
- Le terminal possède ses propres couleurs sombres, même pour la variante Light.
- Le bouton principal possède des couleurs vertes et un relief définis dans les paramètres communs.

ApplicationStyles.xaml contient aussi des couleurs Light par défaut et plusieurs dimensions littérales déjà présentes dans Tokens.xaml. MainMenu.xaml contient ses propres modèles de dessin. Pour le C++, centraliser palettes et dimensions évitera de reproduire cette dispersion.

ThemeManager charge les paramètres communs, puis la palette Base, puis les éventuelles surcharges d'autres designs. Pour Base, il adapte l'accent Windows afin d'obtenir le contraste prévu : la couleur effective n'est donc pas forcément le bleu du fichier palette. Il applique également les attributs DWM de barre de titre et interroge le registre Windows pour le mode système.

La première fenêtre proposée utilise les palettes explicites Base Light et Base Dark. Le bleu des fichiers est la référence initiale ; la reproduction de l'accent système dynamique reste distincte et n'est pas nécessaire à cette première fenêtre. Le rendu WPF et le rendu Qt ne peuvent pas être déclarés identiques sans vérification visuelle.

## Sources exactes

| Fichier | SHA du fichier |
|---|---|
| [App.xaml](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/App.xaml) | `9fa6211d86e4da895fd60aeacc086248b2149e74` |
| [App.xaml.cs](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/App.xaml.cs) | `b599b102e73f85ab16a86a3bca878e447ebcae93` |
| [Services/Theming/ThemeManager.cs](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Services/Theming/ThemeManager.cs) | `0772450badcad138e3e4f174408f3db14b77f0c6` |
| [Dictionaries/Theming/ThemeCatalog.cs](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Dictionaries/Theming/ThemeCatalog.cs) | `248e76d0b67cefae7188fd3eb7b0da3e7e69f78b` |
| [Theme/Base/Light.xaml](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Light.xaml) | `20f00e98d3f550f2041fc081252da7babe498f05` |
| [Theme/Base/Dark.xaml](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Dark.xaml) | `858589b9b08763377f515ef01adb9c6158bc6134` |
| [Theme/Base/Tokens.xaml](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Tokens.xaml) | `e1b3f3f0e8bfa2cbc23ef90e1cc0b1962fab8cd7` |
| [Theme/Base/Views/Styles/ApplicationStyles.xaml](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Views/Styles/ApplicationStyles.xaml) | `c433ea764530af3c26a3f84ad28e135cb694ae8e` |
| [Theme/Base/Views/Windows/Shell/MainWindow.xaml](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Views/Windows/Shell/MainWindow.xaml) | `b2ecb028311648689508254732adb34710229f35` |
| [Theme/Base/Views/Windows/Shell/MainWindow.xaml.cs](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Views/Windows/Shell/MainWindow.xaml.cs) | `5136052e253f447b668adf513f66100e9a11cc7a` |
| [Theme/Base/Views/Windows/Shell/MainWindow.Theming.cs](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Views/Windows/Shell/MainWindow.Theming.cs) | `977c1174c3f9a91c0d84a489bdf033fc7b214df0` |
| [Controllers/MainWindow/MainWindowLifecycleController.cs](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Controllers/MainWindow/MainWindowLifecycleController.cs) | `68955d7b063494d5d85ff18e74d49ea609cb0058` |
| [Theme/Base/Factories/Views/Common/ControlUiFactory.cs](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Factories/Views/Common/ControlUiFactory.cs) | `90708f952267ef34996ab2c52157169e8d37203a` |
| [Theme/Base/Views/Controls/Shell/MainMenu.xaml](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Views/Controls/Shell/MainMenu.xaml) | `34cd0624a61c88cda3a2455d6dbda699f35785c4` |
| [Constants/Views/Shell/MainWindowConstants.cs](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Constants/Views/Shell/MainWindowConstants.cs) | `7d98ceb4e1c4d7314bfcd0ef5ad4c286f1ddd014` |

L'arbre du dépôt a été récupéré sans troncature à cette révision. Seuls les fichiers cités ont été étudiés pour ces conclusions.
