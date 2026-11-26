- [x] Rapprocher les pages médias du thème C#
  - [x] Explorateur
    - [x] Modifier `src/App/Themes/RetroMediaLab/Views/ExplorerPage.cpp` et `ExplorerPage.h` pour placer les commandes à droite, distinguer les informations du média et trier les fichiers.
    - [x] Ajouter les icônes du C# dans `src/App/Themes/RetroMediaLab/FileTypes/` et les intégrer via `src/App/CMakeLists.txt`.
  - [x] Lecture, Écriture et Visualisation
    - [x] Modifier `src/App/Themes/RetroMediaLab/Views/CapturePage.cpp` et `VisualizationPage.cpp` pour replacer les commandes et afficher leurs icônes.
  - [x] Progression et son
    - [x] Modifier `src/App/Themes/RetroMediaLab/Views/StatusBar.cpp` pour réutiliser les événements de présentation des pistes de disquette dans l’Explorateur.
    - [x] Compléter les sessions et vues de progression pour les médias optiques, les disques durs et les cartouches.
    - [x] Raccorder les sons mécaniques au déroulement de la lecture simulée.
    - [x] Modifier `src/MediaEngine/src/Visualization/LinearInspection.cpp` et `src/MediaEngine/include/RetroMediaLab/MediaEngine/LinearVisualization.h` pour lire les plages et vérifier les pistes optiques.
    - [x] Modifier `src/Infrastructure/Media/VisualizationSession.cpp` et `VisualizationSession.h` pour présenter les résultats de lecture et annuler le travail en cours.
    - [x] Modifier `src/App/Themes/RetroMediaLab/Views/CaptureProgressView.cpp`, `CaptureProgressView.h`, `StatusBar.cpp`, `VisualizationPage.cpp`, `MediaGraph.cpp`, `LinearGraph.cpp` et `LinearGraph.h` pour relier la barre et le dessin aux résultats.
    - [x] Ajouter `src/Infrastructure/Media/SimulatedDriveAudio.cpp` et `SimulatedDriveAudio.h`, les déclarer dans `src/Infrastructure/CMakeLists.txt` et les relier à `VisualizationSession.cpp`.
    - [x] Créer puis supprimer `build/verification-media-presentation/presentation.cpp` et ses artefacts après un essai des trois types de supports.
    - [x] Modifier `src/MediaEngine/src/Visualization/BlockProvider.cpp` pour limiter la dernière plage à la longueur réellement stockée d’une cartouche.

Vérifié en Release sur une image ISO, un disque brut MBR et une cartouche SMS de 8 Kio. Les trois passent par un état en attente avant le résultat de lecture. Le rendu PCM produit des échantillons non nuls pour la disquette, le disque dur, le lecteur optique et la cassette. Une piste optique est vérifiée sur son premier secteur stocké. La cartouche ne sollicite aucun lecteur mécanique.

- [x] Corriger le démarrage de la Release
  - [x] Modifier `src/App/Themes/RetroMediaLab/icons.json` pour employer les opérations de tracé acceptées par le chargeur du thème.
  - [x] Créer puis supprimer les journaux temporaires dans `build/verification-theme-startup/` après un démarrage de l’exécutable Release.

Les 17 tracés passent la validation et le lancement natif de la Release initialise la fenêtre principale.

- [x] Stabiliser les écrans de lecture
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/ExplorerPage.cpp` pour superposer le chargement aux trois panneaux.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/MediaLoadingOverlay.cpp` et son en-tête pour conserver les dimensions de la carte pendant les mises à jour.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/CapturePage.cpp`, son en-tête et ses constantes pour rétablir l’agencement précédent de Lecture et Écriture en conservant les boutons aux emplacements demandés.
  - [x] Reporter les icônes d’identité de disquette du C# dans `src/App/Themes/RetroMediaLab/FileTypes/` et `src/App/CMakeLists.txt`.
  - [x] Créer puis supprimer `build/verification-explorer-layout/layout.cpp` et ses artefacts après vérification de la géométrie.

Vérifié en Release sur CrazyCarsIII_Gen4_Tri_Format.scp. Les trois panneaux occupent toute la largeur, la carte conserve sa géométrie sur 237 mises à jour du texte et la liste affiche 27 entrées après la lecture.

- [x] Grouper le tri des entrées de l’Explorateur
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/ExplorerPage.cpp`, `ExplorerPage.h` et `MediaPageConstants.h` pour trier séparément les dossiers et les fichiers, donner moins de largeur au nom et aérer les cellules.
  - [x] Créer puis supprimer un contrôle temporaire dans `build/verification-explorer-sort/` pour vérifier le tri et la largeur des colonnes.

Vérifié en Release sur les 27 entrées du SCP. Les six ordres de tri et la conservation de la sélection ont été confirmés. La colonne Nom mesure 177 pixels sur les 392 disponibles pendant l’essai.

- [x] Reprendre le bouton Exécuter du thème C#
  - [x] Modifier `src/App/Themes/RetroMediaLab/RetroMediaLabStyle.cpp`, `ThemeConstants.h`, `light.json` et `dark.json` pour reprendre le dégradé et les dimensions du bouton principal.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/CapturePage.cpp` pour appliquer ce style aux boutons Exécuter de Lecture et Écriture.
  - [x] Créer puis supprimer un contrôle de taille et de couleur dans `build/verification-execute-button/`.

- [x] Unifier la présentation d’un SCP multiformat
  - [x] Modifier `src/MediaEngine/src/Exploration/Media/Service.cpp` pour identifier le conteneur SCP avant de sélectionner une interprétation.
  - [x] Modifier `src/Infrastructure/Media/ExplorerSession.cpp` et `src/App/Themes/RetroMediaLab/Views/ExplorerPage.cpp` et son en-tête pour réutiliser les formats déjà analysés et afficher le volume sélectionné.
  - [x] Modifier `VisualizationPage.cpp` pour afficher les mêmes formats que l’Explorateur.
  - [x] Modifier `src/Infrastructure/Media/OpeningProgress.h`, `OpeningProgress.cpp`, `VisualizationSession.cpp` et les vues de chargement pour afficher durablement la recherche de format et ralentir le défilement des pistes SCP.
  - [x] Créer puis supprimer le contrôle du SCP triformat dans `build/verification-execute-button/` après avoir vérifié les deux vues.
  - [x] Modifier `src/MediaEngine/src/Exploration/Disk/ProjectionFormats.cpp` pour exposer le volume physique des formats décodés sans catalogue, comme `ExploredDiskImage.SelectFormat` en C#.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/VisualizationPage.cpp` pour réserver la hauteur du bandeau de pistes et du résumé pendant l’ouverture.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/ExplorerPage.cpp` pour réserver les lignes du nom et des formats pendant le chargement.

Validation Release sur CrazyCarsIII_Gen4_Tri_Format.scp. Les deux boutons Exécuter mesurent 164 × 44 à 100 % dans le contrôle Qt et utilisent le dégradé vert du thème C#. L’Explorateur et la Visualisation partagent la même analyse. Les trois choix sont disponibles et les changements de présentation ne relancent pas l’ouverture. AmigaDOS expose 27 entrées à la racine. Les secteurs Atari ST et IBM PC sont reconnus, sans catalogue de fichiers reconnu. Géométrie des panneaux stable sur 10 relevés de l’Explorateur et 401 de la Visualisation pendant cette ouverture.

Le C# dirige les deux boutons Ouvrir vers DiskImageWorkspaceController.LoadAsync. Il affiche la capture pendant la reconnaissance et réutilise le résultat lors d’un changement de présentation. Dans ce port, les deux boutons passent par ExplorerSession::open et VisualizationSession reçoit la capture et le résultat de cette session. Le défilement SCP passe de 30 à 75 ms par piste à la demande de l’utilisateur. La carte de chargement réserve une ligne au format testé et garde chaque libellé affiché au moins 650 ms tant que l’opération continue.
