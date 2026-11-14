# Langues et sélecteurs

Sources C# consultées en lecture seule au commit f770449e957f26ae5f4b7b6130ddc2c52106c7fd :

- [UiLanguageCatalog.cs](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/src/RetroMediaLab.App/Dictionaries/Localization/UiLanguageCatalog.cs), SHA 6123d2425add23d7809038eeab3e9c664e1ba50f : liste des 29 cultures et noms natifs.
- [translate-resx-argos.py](https://github.com/overthetop78/Retro-Media-Lab/blob/f770449e957f26ae5f4b7b6130ddc2c52106c7fd/scripts/tools/translate-resx-argos.py) : mêmes modèles pt pour les deux variantes portugaises, zh pour les deux variantes chinoises, corrections par culture. Les catalogues C# locaux ont été consultés pour comparer les libellés déjà traduits ; les contresens repérés restent corrigés dans les JSON C++.

La première tentative C++ avait choisi pb/zt d'après les codes du dépôt de modèles Argos. L'appel a échoué pour ces codes ; cela ne démontrait pas l'absence des modèles nécessaires à RML. La reprise des correspondances du C# a permis la génération avec les modèles existants pt/zh. Aucun modèle n'a été installé.

Les invariants sont universels : un seul fichier scripts/translations/ignores/common.txt, sans fichiers ni exceptions par langue. Les variantes de traduction ne sont pas des exceptions aux invariants.

Références Qt :

- [QStyle](https://doc.qt.io/qt-6/qstyle.html) : SH_ComboBox_Popup distingue l'affichage en menu de la liste ordinaire.
- [QComboBox, maxVisibleItems](https://doc.qt.io/qt-6/qcombobox.html#maxVisibleItems-prop) : cette limite est ignorée pour certains styles en mode menu.

Le style RetroMediaLab désactive ce mode menu pour les sélecteurs et limite les lignes visibles à 12. La hauteur des éléments de menu est calculée depuis la police, sans réutiliser la hauteur du rectangle fourni par l'appelant. Le test reproduit une hauteur de contenu de 1500 et vérifie que celle-ci ne devient pas une hauteur de ligne.
