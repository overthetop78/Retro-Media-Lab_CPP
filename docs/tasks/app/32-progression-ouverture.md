# Chargement des médias

- [x] Reprendre la progression visible du C#
  - [x] Créer `src/Infrastructure/Media/OpeningProgress.h`, `.cpp` et `OpeningProgressConstants.h`, modifier le CMake et `ExplorerSession.h`/`.cpp` : conserver les étapes/résultats des callbacks MediaEngine dans un état protégé, publier sur le thread GUI et afficher le fichier en cours dès le début.
  - [x] Modifier `VisualizationSession.h`/`.cpp` : exposer la préparation après ouverture et son annulation sans perdre les résultats précédents.
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/MediaLoadingOverlay.h`, `.cpp` et `LoadingConstants.h`, modifier le CMake et `audit_literals.py` : cadre central traduit, détail, progression déterminée/indéterminée et arrêt.
  - [x] Modifier `VisualizationPage.h`/`.cpp`, `ExplorerPage.h`/`.cpp`, `StatusBar.h`/`.cpp` : progression dans le contenu et la barre d’état, abonnements et disparition à la fin/erreur/annulation.
  - [x] Modifier `src/Infrastructure/Media/CaptureSession.h`/`.cpp` : préparation asynchrone de la source d’écriture, progression commune de préparation/lecture/écriture/bilan et annulation avec nettoyage.
  - [x] Modifier `src/Infrastructure/Media/ConversionSession.h`/`.cpp`, `src/App/Themes/RetroMediaLab/Views/CapturePage.h`/`.cpp`, `ConversionPage.h`/`.cpp` et `StatusBar.h`/`.cpp` : raccorder toutes les phases et les cinq pages au cadre et à la barre d’état, sans masquer les confirmations de pertes.
  - [x] Créer temporairement `build/verification-opening-progress/` : vérifier étapes, publication, état occupé et affichage réel des deux indicateurs ; supprimer les fichiers temporaires après utilisation.

  - [x] Modifier `StatusBar.h`/`.cpp`, `LoadingConstants.h`, `CaptureProgressView.h`/`.cpp` : reprendre les colonnes du C# (245/165/125, pistes extensibles, temps et console), profil actif, voyant, deux lignes compactes de cellules/pistes et compteurs ; retirer la jauge générique.
  - [x] Modifier `CaptureSession.h`/`.cpp` et `ConversionSession.h`/`.cpp` : conserver le début d’exécution pour le chronomètre indépendamment du thème ; copier la traduction Status.Profile depuis le C#.

  - [x] Modifier `OpeningProgress.h`/`.cpp` et `ExplorerSession.cpp` : configurer les pistes de la barre depuis le callback de capture réellement chargée et suivre les pistes décodées, sans créer de cellules à partir d’un pourcentage.

  - [x] Modifier `VisualizationSession.h`/`.cpp`, `MediaSessionConstants.h`, `WorkspaceState.cpp`, `ExplorerPage.h`/`.cpp`, `VisualizationPage.cpp`, `SequentialGraph.h`/`.cpp`, `MediaGraph.cpp`, `CaptureProgressView.h`/`.cpp`, `LoadingConstants.h` et `StatusBar.cpp` : reprendre ConfigureSequentialProgress et TapeLoadingPresenter du C#, séparation ordonnée en deux moitiés, couleurs données/silences et compteurs indépendants ; conserver la progression de présentation hors du thème.
