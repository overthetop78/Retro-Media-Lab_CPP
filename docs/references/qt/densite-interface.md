# DensitÃ© et DPI avec Qt Widgets

Sources primaires consultÃ©es le 14 novembre 2026 :

- [Qt High DPI](https://doc.qt.io/qt-6/highdpi.html) : gÃ©omÃ©tries des widgets en pixels indÃ©pendants du pÃ©riphÃ©rique ; Qt applique le facteur de lâ€™Ã©cran. Sous Windows, Qt 6 utilise normalement Per-Monitor DPI Aware V2. Ne pas multiplier une seconde fois ces dimensions par le pourcentage Windows.
- [QScreen](https://doc.qt.io/qt-6/qscreen.html) : `availableGeometry()` dÃ©crit lâ€™espace utilisable ; `devicePixelRatio()` et les signaux de gÃ©omÃ©trie/DPI permettent de suivre un changement dâ€™Ã©cran. La taille physique peut dÃ©pendre des renseignements fournis par le matÃ©riel.
- [Source officielle du backend offscreen Qt](https://github.com/qt/qtbase/blob/dev/src/plugins/platforms/offscreen/qoffscreenintegration.cpp) : le paramÃ¨tre `offscreen:configfile=...` permet de dÃ©clarer plusieurs Ã©crans avec gÃ©omÃ©trie, DPI logique et DPI de base pour les vÃ©rifications hors Ã©cran. Sous Windows, utiliser un chemin relatif pour Ã©viter que le deux-points du lecteur soit interprÃ©tÃ© comme sÃ©parateur dâ€™arguments du plugin.

## DÃ©cision du thÃ¨me RetroMediaLab

Les tailles historiques sont conservÃ©es comme rÃ©fÃ©rences non modifiÃ©es. Ã€ 100 %, le nouveau contenu utilise 70 % de ces tailles ; cette rÃ©duction de 30 % est un choix de prÃ©sentation pour rÃ©pondre Ã  la demande de compacitÃ©. Lâ€™espace logique disponible rÃ©duit encore ce facteur lorsquâ€™il est infÃ©rieur Ã  la rÃ©fÃ©rence 1600 Ã— 900, avec une borne automatique de 75 %. Le choix utilisateur 50/75/100/125/150/200 % multiplie ce rÃ©sultat.

Le facteur Windows reste appliquÃ© par Qt. On ne dÃ©duit pas une diagonale fiable en pouces de la seule rÃ©solution : le rÃ©glage utilisateur permet dâ€™ajuster notamment un portable 14 pouces. Le recalcul utilise chaque fenÃªtre et son Ã©cran, sans redimensionner volontairement sa gÃ©omÃ©trie.

La politique de compacitÃ© appartient au thÃ¨me RetroMediaLab. Les futurs thÃ¨mes peuvent appliquer leur propre prÃ©sentation ; le pourcentage utilisateur est conservÃ© sÃ©parÃ©ment des couleurs et paramÃ¨tres du thÃ¨me.

Correction du 14 novembre : la normalisation des polices hÃ©ritÃ©es sâ€™arrÃªte Ã  la frontiÃ¨re dâ€™une fenÃªtre non encore adaptÃ©e. Un dialogue crÃ©Ã© aprÃ¨s lâ€™application du facteur sur sa fenÃªtre parente ne doit pas voir ses enfants agrandis par une compensation appliquÃ©e deux fois. Les sous-onglets utilisent un facteur propre de 90 %, puis 80 % pour les niveaux imbriquÃ©s, appliquÃ© Ã  leur police, icÃ´ne et espacement.

## Dimensions des sélecteurs et du contenu

Le [code officiel QComboBox](https://raw.githubusercontent.com/qt/qtbase/6.10/src/widgets/widgets/qcombobox.cpp), consulté le 14 novembre 2026, montre que FontChange invalide sizeHint mais pas minimumSizeHint ; StyleChange invalide les deux. Après adaptation des métriques, transmettre StyleChange aux sélecteurs évite de conserver une ancienne hauteur minimale. Les listes courtes calculent leur hauteur sur leur layout adapté, et non sur une mesure figée avant adaptation. Les champs GreaseWeazle reprennent une disposition avec retour à la ligne inspirée du WrapPanel C# et une largeur de saisie de référence de 150.
