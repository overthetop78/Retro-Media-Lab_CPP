# Corrections de la Visualisation et de la Conversion

- [x] Visualisation
  - [x] Modifier `src/Infrastructure/Media/VisualizationSession.cpp` et son en-tête pour sélectionner une présentation sectorielle à partir de l’analyse existante et conserver la révolution lors de la navigation.
  - [x] Modifier `src/MediaEngine/src/Visualization/FluxInspection.cpp` et `src/MediaEngine/include/RetroMediaLab/MediaEngine/FluxVisualization.h` pour inspecter la révolution choisie.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/VisualizationPage.cpp`, son en-tête et ses constantes pour présenter les formats détectés, les icônes de l’Explorateur, le tableau ordonné des secteurs, le repli de l’inspecteur et la légende compacte.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/MediaGraph.cpp`, `FluxGraph.cpp` et leurs en-têtes pour réutiliser le rendu des pistes et mettre en évidence la sélection.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/StatusBar.cpp` et `src/MediaEngine/src/Exploration/Disk/ProjectionFormats.cpp` pour retirer les barres terminées et indiquer que l’espace libre est inconnu lorsque le catalogue est absent.
- [x] Conversion
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/ConversionPage.cpp`, `RetroMediaLabStyle.cpp`, `ThemeConstants.h` et les palettes pour aligner les deux boutons et distinguer leurs couleurs.
- [x] Vérification ciblée
  - [x] Créer puis supprimer `build/verification-visualisation-interactive/` et ses artefacts après le contrôle de l’ouverture SCP, du format, de la sélection et des révolutions.

Vérification Qt hors écran en Release sur CrazyCarsIII_Gen4_Tri_Format.scp. Le retour à Prêt masque les barres dans la barre d’état. La deuxième révolution change le tracé et le détail est issu de cette révolution. Sur la piste 54 face 0, le tableau liste 11 secteurs dans l’ordre numérique et la sélection surligne la piste. La présentation Atari ST utilise les secteurs de la même analyse et le clic sur un secteur change le tracé. Le panneau se replie et se rouvre par sa flèche. Quatre retours à l’onglet avec capture du graphique déjà rendu prennent 33 ms au total pendant ce contrôle. Les deux boutons de Conversion utilisent le même style, en vert pour Exécuter et en bleu pour Migration.

- [x] Refaire la composition après la capture de contrôle
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/VisualizationPage.cpp` et `VisualizationConstants.h` pour donner une ligne pleine largeur au résumé, séparer les commandes et privilégier le graphique.
  - [x] Créer puis supprimer `build/verification-composition-visualisation/` après contrôle Qt de la disposition et des deux captures Crazy Cars III.

Composition vérifiée en Release avec la capture de référence à 840 et 1320 pixels logiques de largeur. Le résumé se répartit sur quatre lignes à 840 pixels et deux à 1320. L’en-tête dispose de la hauteur nécessaire dans les deux cas. À 1320 pixels, le graphique passe de 228 à 386 pixels de hauteur lorsque l’inspecteur est replié.
