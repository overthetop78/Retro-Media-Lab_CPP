# Écarts restants des vues Lecture et SCP

- [x] Reprendre les fiches de Lecture du C#
  - [x] Modifier `src/Infrastructure/Media/CaptureSession.h` et `.cpp` pour conserver les captures de la session avec état et résumé.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/CapturePage.h`, `.cpp` et `CaptureConstants.h` pour afficher les fiches sous le dossier et ouvrir le chemin de chaque fiche.
- [x] Reprendre les éléments visuels SCP du C#
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/FluxGraph.h`, `.cpp`, `MediaGraph.cpp` et `VisualizationConstants.h` pour dessiner les pistes d’attente et aligner la légende sur la qualité des anneaux.
  - [x] Modifier `src/Infrastructure/Media/OpeningProgress.h` et `.cpp`, ainsi que `src/App/Themes/RetroMediaLab/Views/VisualizationPage.h`, `.cpp`, `MediaLoadingOverlay.h` et `.cpp` pour afficher l’identité réelle du SCP dès réception de sa capture et placer la carte en haut des disques.
- [x] Modifier src/App/State/WorkspaceState.cpp, src/App/Shared/Controls/RememberedFileDialog.h et RememberedFileDialogConstants.h pour conserver la destination Lecture, distincte du dossier des sélecteurs.
- [x] Modifier src/Infrastructure/Media/VisualizationSession.h, .cpp et StatusBar.cpp pour démarrer le tracé au chargement de la capture SCP, en parallèle de la reconnaissance.
- [x] Créer puis supprimer build/verification-read-scp/ pour vérifier deux captures réussies, un échec, le dossier restauré et le chevauchement dessin/reconnaissance SCP.
- [x] Reporter les libellés Status.Running et Status.Success des Status.resx C# dans src/Translations/Catalogs/Constants/App/loading-status.json et ses variantes, puis reprendre les trois couleurs de fiche et le sélecteur de destination dédié.
