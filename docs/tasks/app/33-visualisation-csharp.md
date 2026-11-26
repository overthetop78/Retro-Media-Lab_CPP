# Présentation des disques et dossiers des onglets

- [x] Reprendre la visualisation C#
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/FluxVisualization.h` et `src/MediaEngine/src/Visualization/FluxInspection.cpp` pour transmettre les pistes préparées dans l’ordre cylindre puis face.
  - [x] Modifier `src/Infrastructure/Media/VisualizationSession.h`, `.cpp`, `MediaSessionConstants.h` et `src/App/Themes/RetroMediaLab/Views/MediaGraph.cpp`, `FluxGraph.h`, `.cpp` pour révéler les pistes à la cadence C# sur fond transparent.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/VisualizationPage.h`, `.cpp`, `VisualizationConstants.h`, `StatusBar.cpp`, `CaptureProgressView.cpp` et `src/Infrastructure/Media/GwProgressTracker.h` pour placer l’identité, les faces, l’inspecteur et les cellules persistantes en conservant la grille hexadécimale.
- [x] Mémoriser le dossier de chaque onglet équipé d’un sélecteur
  - [x] Créer `src/App/Shared/Controls/RememberedFileDialog.h` et `RememberedFileDialogConstants.h`, adapter `scripts/quality/audit_literals.py` et raccorder les sélecteurs des pages Capture, Conversion, Explorateur, Visualisation, Migration, Préférences et des outils Greaseweazle/KryoFlux.
- [x] Créer puis supprimer build/verification-visualization/ pour contrôler le SCP réel, la grille et les chemins séparés dans une configuration temporaire.
- [x] Modifier src/App/Themes/RetroMediaLab/Views/MediaGraph.h, .cpp et FluxGraph.h, .cpp pour mettre en cache les pistes déjà tracées pendant la révélation.
