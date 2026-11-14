# Densité et DPI avec Qt Widgets

Sources primaires consultées le 14 novembre 2026 :

- [Qt High DPI](https://doc.qt.io/qt-6/highdpi.html) : géométries des widgets en pixels indépendants du périphérique ; Qt applique le facteur de l’écran. Sous Windows, Qt 6 utilise normalement Per-Monitor DPI Aware V2. Ne pas multiplier une seconde fois ces dimensions par le pourcentage Windows.
- [QScreen](https://doc.qt.io/qt-6/qscreen.html) : `availableGeometry()` décrit l’espace utilisable ; `devicePixelRatio()` et les signaux de géométrie/DPI permettent de suivre un changement d’écran. La taille physique peut dépendre des renseignements fournis par le matériel.
- [Source officielle du backend offscreen Qt](https://github.com/qt/qtbase/blob/dev/src/plugins/platforms/offscreen/qoffscreenintegration.cpp) : le paramètre `offscreen:configfile=...` permet de déclarer plusieurs écrans avec géométrie, DPI logique et DPI de base pour les vérifications hors écran. Sous Windows, utiliser un chemin relatif pour éviter que le deux-points du lecteur soit interprété comme séparateur d’arguments du plugin.

## Décision du thème RetroMediaLab

Les tailles historiques sont conservées comme références non modifiées. À 100 %, le nouveau contenu utilise 85 % de ces tailles ; cette réduction de 15 % est un choix de présentation pour répondre à la demande de compacité. L’espace logique disponible réduit encore ce facteur lorsqu’il est inférieur à la référence 1600 × 900, avec une borne automatique de 75 %. Le choix utilisateur 50/75/100/125/150/200 % multiplie ce résultat.

Le facteur Windows reste appliqué par Qt. On ne déduit pas une diagonale fiable en pouces de la seule résolution : le réglage utilisateur permet d’ajuster notamment un portable 14 pouces. Le recalcul utilise chaque fenêtre et son écran, sans redimensionner volontairement sa géométrie.

La politique de compacité appartient au thème RetroMediaLab. Les futurs thèmes peuvent appliquer leur propre présentation ; le pourcentage utilisateur est conservé séparément des couleurs et paramètres du thème.
