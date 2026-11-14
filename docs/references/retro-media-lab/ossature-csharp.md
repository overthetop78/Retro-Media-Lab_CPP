# Ossature C# consultée

Référence en lecture seule : [Retro-Media-Lab, f770449e957f26ae5f4b7b6130ddc2c52106c7fd](https://github.com/overthetop78/Retro-Media-Lab/tree/f770449e957f26ae5f4b7b6130ddc2c52106c7fd).

- [Theme/Base/Views/Windows/Shell/MainWindow.xaml](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Views/Windows/Shell/MainWindow.xaml) : b2ecb028311648689508254732adb34710229f35.
- [Theme/Base/Views/Controls/Shell/MainMenu.xaml](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Views/Controls/Shell/MainMenu.xaml) : 34cd0624a61c88cda3a2455d6dbda699f35785c4.
- [Theme/Base/Views/Styles/ApplicationStyles.xaml](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Views/Styles/ApplicationStyles.xaml) : c433ea764530af3c26a3f84ad28e135cb694ae8e.
- [TerminalSection.xaml](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Views/Controls/Shell/TerminalSection.xaml) : 4c8c65ba1b1daa37945e9f8f95dac3b3f9d93bb9.
- [TerminalSection.xaml.cs](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Views/Controls/Shell/TerminalSection.xaml.cs) : b79abfe854200c114e086f9fec9c0bb96ee64bee.
- [ApplicationStatusBar.xaml](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Views/Controls/Shell/ApplicationStatusBar.xaml) : e169da6cd20b2f0da60fe0ed79c1ffd53e612576.
- [ApplicationStatusBar.xaml.cs](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Theme/Base/Views/Controls/Shell/ApplicationStatusBar.xaml.cs) : be6a2d954a5dc0b8aea02168fa937fab97d84c5d.

## Données utiles au portage

Ordre des pages : lecture, écriture, conversion, visualisation, explorateur, outils, émulation. MainWindow utilise un TabControl, des marges 16,10,16,0, un séparateur de 4 unités et une console de 190 unités. Le cadre central est indépendant des blocs de contenu métier.

Le menu reprend Options (préférences, mises à jour, historique, diagnostics, matériel), Émulation (préférences) et Aide (documentation, à propos). Aucun sélecteur permanent de langue ou de variante n'est présent dans cette barre.

MainTabHeader utilise des icônes monochromes (lecture, crayon, synchronisation, visualisation, dossier, outils, manette), taille 17 et espacement 8. Le C++ dessine ces symboles sans dépendre de Segoe MDL2 Assets. ApplicationStyles définit des marges internes 22,11, un écart de 3 entre onglets, des coins supérieurs de rayon 9 et une ligne active de 3.

La console C# affiche une commande et sa sortie en lecture seule ; elle ne constitue pas un interpréteur interactif. Elle possède une copie locale et des couleurs dédiées : fond #0B0F14, bordure #303A46, sortie #D9DEE7, commande #B7F7C3, prompt #54E88B.

La capture fournie par l'utilisateur sert de référence de disposition : console repliée, menus sans sélecteurs, zone centrale encadrée. Les blocs de lecture et le bouton Exécuter visibles dans cette capture ne sont pas portés à cette étape. Les couleurs des palettes sources et l'accent effectif de l'application C# peuvent différer : ThemeManager adapte l'accent système (voir la référence graphique déjà enregistrée).
