# Portage du socle Émulation

Le portage s'appuie sur la branche C# `osd-support` et couvre le socle commun `Emulation`, `VideoPresentation` et leur intégration dans `App`. Les modules de machines `Emulation.xxxx` sont reportés à une étape ultérieure.

- [x] Contrats et services communs
  - [x] Créer `src/Emulation/CMakeLists.txt` et les fichiers du SDK pour les modules, machines, sessions, entrées et flux vidéo et audio.
  - [x] Créer dans `src/Emulation` le chargement des modules, la gestion des sessions et l'accès aux services médias.
- [x] Présentation vidéo
  - [x] Créer `src/VideoPresentation/CMakeLists.txt` et les fichiers pour la configuration, le rendu et l'OSD.
- [x] Intégration
  - [x] Modifier `CMakeLists.txt` et `src/App/CMakeLists.txt` pour les deux bibliothèques.
  - [x] Modifier `src/App/State/WorkspaceState.h` et `src/App/Themes/RetroMediaLab/Views/WorkspaceTabs.cpp` et créer la vue Émulation.
- [x] Précisions du premier bloc
  - [x] Créer `src/Emulation/include/RetroMediaLab/Emulation/Sdk.h` pour le SDK natif et `Host.h` pour le propriétaire des modules et sessions.
  - [x] Créer `src/Emulation/src/Host.cpp` pour valider `Modules/<id>/module.json`, garder les bibliothèques chargées selon la durée de vie des machines et piloter leur cycle de vie.
  - [x] Créer `src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h` et `src/VideoPresentation/src/Presentation.cpp` pour les pixels, proportions, trames entrelacées, profils et affichage plein écran.
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/EmulationPage.h` et `EmulationPage.cpp` pour les modules, machines, configurations et contrôles de session.
  - [x] Créer les catalogues `src/Translations/Catalogs/{Constants,Languages/*}/App/emulation.json` à partir des ressources amont et compléter les libellés de ce bloc.
- [x] API commune des médias émulés
  - [x] Modifier `src/Emulation/include/RetroMediaLab/Emulation/Sdk.h` pour exposer des contrats disque et optique indépendants de MediaEngine.
  - [x] Créer `src/Emulation/src/MediaPreparation.h` et `src/Emulation/src/MediaPreparation.cpp` pour adapter les périphériques médias au moteur déjà porté, modifier `src/Emulation/src/Host.cpp` et `src/Emulation/CMakeLists.txt` pour les utiliser.
- [x] Réglages vidéo et plein écran dans l'application
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` pour les réglages vidéo persistants et les commandes de l'OSD.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/MainMenu.h`, `MainMenu.cpp` et `src/App/Themes/RetroMediaLab/RetroMediaLabLayout.cpp` pour ouvrir la page depuis le menu.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/WorkspaceTabs.h` et `EmulationPage.h` pour le point d'entrée des réglages.
- [x] Validation et distribution
  - [x] Créer puis retirer `build/verification-emulation-native` pour exercer un module de test temporaire et les trames de rendu.
- [x] Configuration générique des machines
  - [x] Modifier `src/Emulation/include/RetroMediaLab/Emulation/Sdk.h`, `Host.h` et `src/Emulation/src/Host.cpp` pour partager les descriptions de blocs, de champs et de valeurs.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` pour afficher et enregistrer les paramètres déclarés par un module.

- [x] Enveloppe des sauvegardes et compatibilité des profils
  - [x] Créer `src/Emulation/include/RetroMediaLab/Emulation/StateFile.h` et `src/Emulation/src/StateFile.cpp` pour lire et écrire le format commun C# et vérifier les empreintes et les identités.
  - [x] Modifier `src/Emulation/CMakeLists.txt` pour compiler le service.
  - [x] Modifier `src/VideoPresentation/src/Presentation.cpp` pour utiliser les clés JSON du profil C#.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` pour réafficher la surface après son déplacement en plein écran.

## Couverture restante de la branche osd-support

- [ ] Compléter les contrats et services exportés dans `src/Emulation/include/RetroMediaLab/Emulation` pour l'exécution des machines, le protocole des cœurs hébergés, les firmwares, les adaptateurs et les contrôleurs.
- [ ] Créer dans `src/VideoPresentation/src` les moteurs matériels, le traitement du signal, les effets d'affichage et les autres modes vidéo de la branche source.
- [ ] Compléter `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` avec le transport des médias, l'affectation des contrôleurs, le retour de force et les commandes OSD restantes.
- [x] Compléter les ressources d'Émulation dans `src/Translations/Catalogs/Languages` avec les libellés propres au portage absents des catalogues C#.
- [x] Bandeau des périphériques de session
  - [x] Modifier `src/Emulation/include/RetroMediaLab/Emulation/Sdk.h` pour exposer l'état du transport de cassette, l'opération active et les commandes prises en charge.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` pour afficher les périphériques de chaque machine en bas de la vue et reprendre les règles du C# de présentation des commandes de cassette.
  - [x] Créer `src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-tape.json` à partir des libellés de la branche source.

- [x] Création et enregistrement d'une cassette
  - [x] Modifier `src/Emulation/include/RetroMediaLab/Emulation/Sdk.h` et `Host.h`, ainsi que `src/Emulation/src/Host.cpp` et `MediaPreparation.{h,cpp}` pour exposer les extensions acceptées, les paramètres PCM et la création via MediaEngine.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` pour choisir un fichier, configurer le PCM RAW, le monter et lancer le transport.
  - [x] Créer puis retirer `build/verification-emulation-tape` pour vérifier les formats de cassette vierge, le filtrage des extensions et le nettoyage.

- [x] Traitement logiciel des trames et de l'image
  - [x] Créer `src/VideoPresentation/src/SoftwareProcessing.{h,cpp}` pour composer les champs et appliquer les ajustements en lumière linéaire.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour intégrer l'historique et les réglages du profil.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et créer les catalogues `App/emulation-video-processing.json` pour exposer les réglages dans App.
  - [x] Créer puis retirer `build/verification-emulation-video` pour contrôler les trames, les ajustements et la persistance des profils.

- [x] Présentation OpenGL
  - [x] Créer `src/VideoPresentation/src/OpenGlSurface.{h,cpp}` pour présenter les images sur un contexte OpenGL Qt.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour utiliser le moteur configuré et conserver la composition commune.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et les catalogues `App/emulation-video-processing.json` pour choisir le moteur vidéo.
  - [x] Modifier `CMakeLists.txt` et `scripts/local-building/build.ps1` pour compiler et distribuer les dépendances Qt OpenGL.
  - [x] Créer puis retirer `build/verification-emulation-opengl` pour vérifier le tampon rendu et la sélection du moteur.

- [x] Présentation Direct3D 11
  - [x] Créer `src/VideoPresentation/src/Direct3DSurface.{h,cpp}` pour présenter les trames et les surimpressions sur une chaîne d'affichage sous Windows.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour sélectionner Direct3D 11 et intégrer le rendu au profil.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et les catalogues `App/emulation-video-processing.json` pour exposer le choix sous Windows.
  - [x] Créer puis retirer `build/verification-emulation-d3d11` pour vérifier la création de la chaîne, le changement de taille et le choix du moteur.

- [x] Simulation des connexions et normes analogiques
  - [x] Créer `src/VideoPresentation/src/AnalogSignal.{h,cpp}` pour les filtres RGB SCART, composantes, S-Video, composite, RF, PAL, NTSC et SECAM.
  - [x] Modifier `src/VideoPresentation/src/SoftwareProcessing.{h,cpp}` et `src/VideoPresentation/{CMakeLists.txt,src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour appliquer les filtres et enregistrer le profil.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et créer les catalogues `App/emulation-video-signal.json` pour les réglages.
  - [x] Créer puis retirer `build/verification-emulation-signal` pour vérifier les filtres et le profil.

- [x] Simulation logicielle CRT
  - [x] Créer `src/VideoPresentation/src/CrtProcessing.{h,cpp}` pour la synchronisation, les décodeurs, la courbure, le faisceau et les masques de phosphore.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/Presentation.cpp,src/SoftwareProcessing.{h,cpp},include/RetroMediaLab/VideoPresentation/Presentation.h}` pour utiliser le profil CRT et la taille de sortie.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et créer les catalogues `App/emulation-video-crt.json` pour les réglages CRT.
  - [x] Créer puis retirer `build/verification-emulation-crt` pour vérifier la courbure, les phosphores, le décodage et les profils.

- [x] Connexions physiques et palettes CRT
  - [x] Créer `src/VideoPresentation/src/CrtConnection.{h,cpp}` pour porter la dégradation et les palettes de `FilterCrtConnection.cs`.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/SoftwareProcessing.cpp,src/CrtProcessing.cpp,src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour traiter les connexions avant la conversion en lumière linéaire et enregistrer leurs réglages.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et les catalogues `App/emulation-video-crt.json` pour exposer la dégradation et les palettes.
  - [x] Créer puis retirer `build/verification-emulation-crt-connection` pour vérifier les filtres et les palettes sur une mire.

- [x] Écrans à pixels fixes
  - [x] Créer `src/VideoPresentation/src/FixedPixelProcessing.{h,cpp}` pour porter les sous-pixels, la grille, l’éclairage LCD/LED/OLED et les filtres temporels du C#.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/SoftwareProcessing.{h,cpp},src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour présenter et sauvegarder les profils d’écran.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et créer les catalogues `App/emulation-video-fixed-pixel.json` pour exposer les réglages.
  - [x] Créer puis retirer `build/verification-emulation-fixed-pixel` pour vérifier le rendu des trois technologies, le temps de réponse et les profils.

- [x] Effets vidéo stylistiques
  - [x] Créer `src/VideoPresentation/src/StylisticProcessing.{h,cpp}` pour les filtres VHS, aberration chromatique, halo, sépia et grain issus du C#.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/SoftwareProcessing.cpp,src/FixedPixelProcessing.{h,cpp},src/CrtProcessing.{h,cpp},src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour l’ordre des filtres et la sauvegarde.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et ajouter les catalogues `App/emulation-video-stylistic.json` pour les réglages.
  - [x] Créer puis retirer `build/verification-emulation-stylistic` pour comparer les effets sur une mire et vérifier le rechargement du profil.

- [x] Restauration vidéo
  - [x] Créer `src/VideoPresentation/src/RestorationProcessing.{h,cpp}` pour le détramé, la réduction des bandes et la récupération de détails de `osd-support`.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/ColorTransfer.h,src/FixedPixelProcessing.cpp,src/SoftwareProcessing.cpp,src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour l’ordre des filtres et la sauvegarde.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et créer les catalogues `App/emulation-video-restoration.json` pour les réglages.
  - [x] Créer puis retirer `build/verification-emulation-restoration` pour vérifier les filtres et les profils.

- [x] Écran plasma
  - [x] Créer `src/VideoPresentation/src/PlasmaProcessing.{h,cpp}` pour les huit traitements plasma de `osd-support`.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/SoftwareProcessing.{h,cpp},src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour le rendu et la sauvegarde du profil plasma.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et créer les catalogues `App/emulation-video-plasma.json` pour les réglages.
  - [x] Créer puis retirer `build/verification-emulation-plasma` pour vérifier le rendu, la persistance et le profil.

- [x] Écran vectoriel
  - [x] Créer `src/VideoPresentation/src/VectorProcessing.{h,cpp}` pour le faisceau, les phosphores, le halo et la rémanence.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/SoftwareProcessing.{h,cpp},src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour le rendu et les profils.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et créer les catalogues `App/emulation-video-vector.json` pour les réglages.
  - [x] Créer puis retirer `build/verification-emulation-vector` pour vérifier le tracé et la rémanence ainsi que la sauvegarde.

- [x] Afficheur fluorescent
  - [x] Créer `src/VideoPresentation/src/VfdProcessing.{h,cpp}` pour les cellules, le verre, le phosphore, le halo et la rémanence.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/SoftwareProcessing.{h,cpp},src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour le rendu et les profils.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et créer les catalogues `App/emulation-video-vfd.json` pour les réglages.
  - [x] Créer puis retirer `build/verification-emulation-vfd` pour vérifier le rendu et le profil.

- [x] Matrice LED
  - [x] Créer `src/VideoPresentation/src/LedMatrixProcessing.{h,cpp}` pour la géométrie et l’émission des cellules.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/SoftwareProcessing.cpp,src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour le rendu, la sélection de technologie et le profil.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et créer les catalogues `App/emulation-video-led-matrix.json` pour les réglages.
  - [x] Créer puis retirer `build/verification-emulation-led-matrix` pour vérifier la géométrie et le profil.

- [x] Affichage matriciel à points
  - [x] Créer `src/VideoPresentation/src/{MatrixCells.h,DotMatrixProcessing.h,DotMatrixProcessing.cpp}` et modifier `LedMatrixProcessing.cpp` pour partager le calcul des cellules et porter le rendu et la réponse temporelle.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/SoftwareProcessing.{h,cpp},src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour intégrer la technologie et le profil.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et créer les catalogues `App/emulation-video-dot-matrix.json` pour les réglages.
  - [x] Créer puis retirer `build/verification-emulation-dot-matrix` pour vérifier les modes réfléchissant et émissif, la réponse et le profil.

- [x] Afficheur à segments
  - [x] Créer `src/VideoPresentation/src/SegmentDisplayProcessing.{h,cpp}` pour la géométrie, l’activation des segments et la rémanence.
  - [x] Modifier `src/VideoPresentation/{CMakeLists.txt,src/SoftwareProcessing.{h,cpp},src/Presentation.cpp,include/RetroMediaLab/VideoPresentation/Presentation.h}` pour le rendu et le profil.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et créer les catalogues `App/emulation-video-segment-display.json` pour les réglages.
  - [x] Créer puis retirer `build/verification-emulation-segment-display` pour vérifier le rendu et le profil.

- [x] Papier électronique et projection
  - [x] Modifier `src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h` et `src/VideoPresentation/src/Presentation.cpp` pour les deux profils et leur persistance.
  - [x] Créer `src/VideoPresentation/src/PaperProjectionProcessing.h` et `PaperProjectionProcessing.cpp` et les raccorder à `SoftwareProcessing.cpp`, `SoftwareProcessing.h` et `src/VideoPresentation/CMakeLists.txt`.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et créer `src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-video-paper-projection.json` pour les réglages.

- [x] OSD en plein écran
  - [x] Créer `src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h` et `src/VideoPresentation/src/FullscreenOsd.cpp` et modifier `src/VideoPresentation/CMakeLists.txt` pour superposer le menu, ses sous-menus et les raccourcis de navigation à l'image vidéo.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et créer `src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-osd.json` pour raccorder la session et les commandes déjà disponibles.

- [x] Sauvegarde et chargement rapides
  - [x] Modifier `src/Emulation/include/RetroMediaLab/Emulation/Host.h` et `src/Emulation/src/Host.cpp` pour exposer les commandes avec un fichier .gwas par module et configuration.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et les catalogues `src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-osd.json` pour les commandes de session, les raccourcis et les entrées de l'OSD.
  - [x] Créer puis supprimer la sonde et le module simulé dans `build/verification-quick-states` pour vérifier les états, les chemins et le chargement après réouverture.

- [x] Réglages vidéo dans l'OSD
  - [x] Modifier `src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h` et `src/VideoPresentation/src/FullscreenOsd.cpp` pour les réglages en ligne, la navigation, l'aperçu et la restauration en cas d'échec de sauvegarde.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et les catalogues `src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-osd.json` pour utiliser les commandes du profil dans l'OSD.
  - [x] Créer puis supprimer une sonde dans `build/verification-osd-editor` pour vérifier l'édition, la navigation et le retour à la dernière valeur enregistrée.

- [x] Réglages des modules dans l'OSD
  - [x] Modifier `src/Emulation/include/RetroMediaLab/Emulation/Sdk.h`, `Host.h` et `src/Emulation/src/Host.cpp` pour enregistrer les changements, filtrer les options à chaud et employer la configuration au prochain démarrage.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et les catalogues `src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-osd.json` pour les groupes et choix audio, vidéo et machine.
  - [x] Créer puis supprimer une sonde dans `build/verification-osd-module-options` pour vérifier les changements à chaud et la réutilisation de la configuration.

- [x] Médias montés et recréation de la machine
  - [x] Modifier `src/Emulation/include/RetroMediaLab/Emulation/Sdk.h`, `Host.h` et `src/Emulation/src/Host.cpp` pour conserver les médias par session, les monter avant démarrage et les enregistrer via les modules qui fournissent ce contrat.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` pour passer par la session à l'insertion, à l'éjection et à l'enregistrement sur cassette.
  - [x] Créer puis supprimer la sonde dans `build/verification-mounted-media` pour vérifier le transfert et la reprise après un échec.

- [x] Champs d’information et curseurs des modules
  - [x] Modifier `src/Emulation/include/RetroMediaLab/Emulation/Sdk.h`, `Host.h` et `src/Emulation/src/Host.cpp` pour transmettre les plages et unités déclarées.
  - [x] Modifier `src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h` et `src/VideoPresentation/src/FullscreenOsd.cpp` pour afficher le statut des champs informatifs.
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/ModuleNumericSetting.h`, modifier `EmulationPage.cpp` et les catalogues `src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-osd.json` pour les curseurs, valeurs automatiques et unités.
  - [x] Créer puis supprimer la sonde dans `build/verification-osd-numeric` pour vérifier les plages et valeurs persistées.

- [x] Dépendances entre les réglages des modules
  - [x] Modifier `src/Emulation/include/RetroMediaLab/Emulation/Sdk.h`, `Host.h` et `src/Emulation/src/Host.cpp`, et créer `src/Emulation/include/RetroMediaLab/Emulation/SettingsRules.h` pour partager l'évaluation des exclusions et de la visibilité.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` pour appliquer ces règles dans le dialogue et l'OSD.
  - [x] Créer puis supprimer une sonde dans `build/verification-settings-rules` pour vérifier les exclusions symétriques, les dépendances de visibilité et la conservation des valeurs indépendantes.

- [x] Saisie numérique et pas fin des curseurs
  - [x] Modifier `src/Emulation/include/RetroMediaLab/Emulation/Sdk.h`, `Host.h` et `src/Emulation/src/Host.cpp` pour exposer les options de saisie numérique.
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/ModuleEditableSlider.h` et modifier `EmulationPage.cpp` pour relier saisie, curseur, boutons de pas et valeur automatique au brouillon.
  - [x] Créer puis supprimer une sonde dans `build/verification-editable-slider` pour vérifier le pas et la bascule automatique.

- [x] Manettes Windows et navigation OSD
  - [x] Créer `src/Platform/include/RetroMediaLab/Platform/Gamepads.h`, `src/Platform/Windows/Gamepads.cpp` et `src/Platform/Unix/Gamepads.cpp`, puis modifier `src/Platform/CMakeLists.txt` pour exposer les états GameInput.
  - [x] Modifier `src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h` et `src/VideoPresentation/src/FullscreenOsd.cpp` pour partager la navigation clavier et manette.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et `src/Emulation/include/RetroMediaLab/Emulation/Sdk.h` pour router les entrées et filtrer les commandes maintenues à la fermeture de l'OSD.

- [x] Raccourcis clavier du socle
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/EmulationKeyboardShortcuts.h` pour déclarer les combinaisons de `EmulationShortcutDefaults` applicables aux commandes portées.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` pour relier ces raccourcis à la session et au plein écran.

- [x] Capture relative de la souris
  - [x] Modifier `src/Emulation/include/RetroMediaLab/Emulation/Sdk.h` pour exposer la politique de capture du module et la molette horizontale.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` et `EmulationKeyboardShortcuts.h` pour router la souris capturée, libérer sa capture à la perte de focus, gérer la transition OSD et raccorder son raccourci.

- [x] Réglages de la machine ouverte
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` pour réutiliser l’éditeur de la configuration sur la session ouverte, charger sa dernière version, enregistrer via la session et y raccorder le bouton, le menu et Ctrl+Maj+F5.

- [x] Préférences et fichiers des sessions
  - [x] Modifier `src/Infrastructure/State/PreferencesStore.h`, `PreferencesStore.cpp`, `src/App/Bootstrap/Application.cpp` et `src/App/State/WorkspaceState.{h,cpp}` pour enregistrer les statistiques plein écran et le dossier des captures.
  - [x] Modifier `src/App/Shared/Controls/RememberedFileDialog.h` et `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` pour mémoriser les dossiers par machine et support, utiliser le dossier de captures et dissocier les statistiques plein écran.

- [x] Raccourcis personnalisés depuis les préférences
  - [x] Modifier `src/Infrastructure/State/PreferencesStore.{h,cpp}`, `src/App/Bootstrap/Application.cpp` et `src/App/State/WorkspaceState.h` pour conserver les affectations d’émulation dans preferences.json.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationKeyboardShortcuts.h` et `EmulationPage.cpp` pour résoudre les affectations clavier des commandes, y compris les alternatives de l’OSD.
  - [x] Modifier `src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h` et `src/VideoPresentation/src/FullscreenOsd.cpp` pour déléguer le clavier aux affectations de la session.
  - [x] Créer puis supprimer une sonde dans `build/verification-shortcut-bindings` pour vérifier le pavé numérique, la désactivation et les alternatives.

- [x] Éditeur des raccourcis d’émulation
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/EmulationPreferencesDialog.{h,cpp}` et modifier `src/App/CMakeLists.txt` pour présenter, rechercher, modifier et réinitialiser les affectations.
  - [x] Modifier `src/App/State/WorkspaceState.{h,cpp}`, `src/App/Bootstrap/Application.cpp` et `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` pour enregistrer et rafraîchir les raccourcis des sessions ouvertes, et ouvrir les préférences depuis le menu.
  - [x] Créer `src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-shortcuts.json` avec les libellés repris du C#.

- [x] Raccourcis clavier réservés
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationKeyboardShortcuts.h` et `EmulationPreferencesDialog.cpp` pour refuser les raccourcis réservés de `ReservedKeyboardChordCatalog` et les conflits de préfixe Qt.
  - [x] Modifier les catalogues `src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-shortcuts.json` avec le libellé C# « Réservé ».

- [x] Affectations souris et manette des sessions
  - [x] Modifier `src/Platform/include/RetroMediaLab/Platform/Gamepads.h` et `src/Platform/Windows/Gamepads.cpp` pour exposer l’identité GameInput et les contrôles des manettes.
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/EmulationPhysicalShortcuts.h` pour résoudre les affectations souris et manette de osd-support.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp` pour déclencher les commandes du socle au front d’activation et retirer les affectations manette ajoutées par défaut.
  - [x] Créer puis supprimer une sonde dans `build/verification-physical-shortcuts` pour vérifier la sélection de la manette, les axes et la molette.

- [x] Capture souris et manette dans l’éditeur
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPhysicalShortcuts.h` pour partager la nomenclature entre capture et exécution.
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/EmulationShortcutCapture.h` et modifier `EmulationPreferencesDialog.cpp` pour enregistrer une entrée physique, afficher et effacer son affectation et signaler les doublons.
  - [x] Modifier les catalogues `src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-shortcuts.json` avec le libellé C# d’affectation.
  - [x] Créer puis supprimer une sonde Qt dans `build/verification-shortcut-capture` pour vérifier le dialogue et la détection de manette.

- [x] Combinaisons des entrées d’émulation
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationPhysicalShortcuts.h` et `EmulationPage.cpp` pour reconnaître les entrées simultanées clavier, souris et manette.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EmulationShortcutCapture.h` et `EmulationPreferencesDialog.cpp` pour capturer jusqu’au relâchement et valider les combinaisons équivalentes.
  - [x] Créer puis supprimer une sonde Qt dans `build/verification-mixed-shortcuts` pour vérifier la capture et le relâchement des combinaisons.

- [x] Contrôles bruts GameInput
  - [x] Modifier `src/Platform/include/RetroMediaLab/Platform/Gamepads.h` et `src/Platform/Windows/Gamepads.cpp` pour lire les axes, boutons et commutateurs bruts, avec ou sans mappage standard.
  - [x] Modifier `src/Emulation/include/RetroMediaLab/Emulation/Sdk.h`, `src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp`, `EmulationPhysicalShortcuts.h` et `EmulationShortcutCapture.h` pour transmettre les contrôles, les affecter et les filtrer à la sortie de l’OSD.
  - [x] Créer puis supprimer une sonde dans `build/verification-raw-gameinput` pour vérifier les fronts, la direction et l’identifiant des contrôles bruts.

- [x] Repli Windows pour les manettes hors GameInput
  - [x] Modifier `src/Platform/Windows/Gamepads.cpp` et `src/Platform/CMakeLists.txt` pour lire les périphériques RawGameController, conserver leurs identifiants C# et les dédupliquer par rapport à GameInput.

- [x] Mappage par modèle des manettes hors GameInput
  - [x] Reprendre gamecontrollerdb.txt et sa licence dans src/Platform/Windows/ControllerDatabase depuis osd-support.
  - [x] Modifier src/Platform/CMakeLists.txt et src/Platform/Windows/Gamepads.cpp pour embarquer cette base et traduire les contrôles bruts en commandes standard.
  - [x] Créer puis supprimer une sonde dans build/verification-controller-database pour vérifier la ressource Qt et un mappage Windows réel avec des entrées simulées.

- [x] Rapports bruts des manettes GameInput
  - [x] Créer src/Platform/Windows/GameInputReportControls.h et modifier src/Platform/Windows/Gamepads.cpp pour décoder les éléments HID décrits par GameInput v0 quand les tableaux de contrôles sont absents.
  - [x] Créer puis supprimer une sonde dans build/verification-gameinput-report pour vérifier les axes signés, les boutons, les chapeaux et les identifiants de rapport.

- [x] Retour de force des sessions
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h pour exposer les sorties de vibration facultatives.
  - [x] Modifier src/Platform/include/RetroMediaLab/Platform/Gamepads.h, Windows/Gamepads.cpp et Unix/Gamepads.cpp pour appliquer les quatre moteurs pris en charge et les arrêter à la fin de leur utilisation.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour transmettre les sorties de la session active et arrêter les moteurs au retrait de la session.

- [x] Routage des effets de vibration par port
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/ControllerFeedback.h et src/Emulation/src/ControllerFeedback.cpp pour normaliser et mémoriser les effets des cœurs puis résoudre les affectations par port.
  - [x] Modifier src/Emulation/CMakeLists.txt pour compiler et exporter ces services dans Emulation.
  - [x] Créer puis supprimer une sonde dans build/verification-controller-feedback pour vérifier les limites de port, la combinaison des effets et leur remise à zéro.

- [x] Choix des périphériques de vibration
  - [x] Modifier src/Platform/include/RetroMediaLab/Platform/Gamepads.h, src/Platform/Windows/Gamepads.cpp et src/Platform/Unix/Gamepads.cpp pour énumérer les périphériques et les moteurs disponibles.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/ControllerFeedback.h et src/Emulation/src/ControllerFeedback.cpp pour reproduire les affectations par défaut et la suppression d’une affectation depuis le choix du périphérique.

- [x] Éditeur des affectations de vibration
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/ControllerFeedback.h et Host.h ainsi que src/Emulation/src/Host.cpp pour exposer les ports fournis par le module et enregistrer les affectations avec la configuration.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour afficher la sélection simple et les moteurs par commande, en préservant les manettes débranchées.
  - [x] Créer les catalogues App/emulation-controller-feedback.json dans Constants et Languages pour reprendre les libellés des catalogues C#.

- [x] Codecs des cœurs hébergés
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/HostProtocol.h et src/Emulation/src/HostProtocol.cpp pour porter les en-têtes, chaînes, blocs binaires, vibrations, vidéo et audio du protocole C#.
  - [x] Modifier src/Emulation/CMakeLists.txt pour intégrer ces codecs à Emulation.dll.
  - [x] Créer puis supprimer une sonde dans build/verification-host-protocol pour vérifier les octets attendus, la lecture successive et le rejet des messages invalides.

- [x] Entrées et statuts des cœurs hébergés
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostProtocol.h et src/Emulation/src/HostProtocol.cpp pour porter les entrées clavier, souris et manettes ainsi que les statuts audio, lecteur et voyants.
  - [x] Créer puis supprimer une sonde dans build/verification-host-input pour vérifier la disposition du flux, les bornes et la lecture successive.

- [x] Vidéo partagée des cœurs hébergés
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostProtocol.h et src/Emulation/src/HostProtocol.cpp pour créer le segment nommé, alterner les deux emplacements et lire les métadonnées du C#.
  - [x] Créer puis supprimer une sonde dans build/verification-host-video pour vérifier les deux emplacements, les octets du message, les bornes et la fermeture du segment.

- [x] Canal local et durée de vie des cœurs hébergés
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/HostedCoreTransport.h et src/Emulation/src/HostedCoreTransport.cpp pour lancer le cœur, lui transmettre les noms du canal et du segment vidéo et échanger les réponses binaires bornées.
  - [x] Modifier src/Emulation/CMakeLists.txt pour intégrer le transport à Emulation.dll.
  - [x] Créer puis supprimer une sonde dans build/verification-host-transport pour vérifier deux échanges avec un processus enfant, l’ouverture de la mémoire vidéo par le processus enfant et la fermeture sur une réponse invalide.

- [x] Commandes du client des cœurs hébergés
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostedCoreTransport.h et src/Emulation/src/HostedCoreTransport.cpp pour porter les codes, les arguments communs et les réponses du client C#.
  - [x] Créer puis supprimer une sonde dans build/verification-host-commands pour comparer les octets échangés et vérifier les réponses de succès et d’erreur.

- [x] Mémoire vidéo redimensionnable des cœurs hébergés
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostProtocol.h et src/Emulation/src/HostProtocol.cpp pour créer, annoncer et ouvrir les générations de segments vidéo du C#.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostedCoreTransport.h et src/Emulation/src/HostedCoreTransport.cpp pour démarrer le transport avec le mode vidéo attendu par le cœur.
  - [x] Créer puis supprimer une sonde dans build/verification-resizable-video pour vérifier les métadonnées, deux tailles et le rejet d’un segment hors session.

- [x] Initialisation et trames du client des cœurs hébergés
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/HostedCoreSession.h et src/Emulation/src/HostedCoreSession.cpp pour piloter le lancement, l’initialisation, les entrées, les sorties de trame et la fermeture.
  - [x] Modifier src/Emulation/CMakeLists.txt pour intégrer le client commun.
  - [x] Créer puis supprimer build/verification-host-session pour exercer le cycle de la session via un hôte temporaire.

- [x] Présentation Vulkan
  - [x] Créer src/VideoPresentation/src/VulkanSurface.h et VulkanSurface.cpp pour présenter les trames sur un moteur Vulkan avec le cadre et l’OSD existants.
  - [x] Modifier src/VideoPresentation/CMakeLists.txt, src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour intégrer le moteur et gérer son indisponibilité.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et le catalogue Constants pour proposer Vulkan dans les deux interfaces de réglage.
  - [x] Créer puis supprimer build/verification-vulkan-video pour vérifier la présentation, les couleurs et le passage d’un moteur à l’autre sous Windows.

- [x] Services de firmware du SDK
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/Firmware.h et src/Emulation/src/Firmware.cpp pour les candidats, observations, sélections, parcours et empreintes.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h, src/Emulation/src/Host.cpp et src/Emulation/CMakeLists.txt pour relier la gestion aux modules qui déclarent cette capacité.
  - [x] Créer puis supprimer build/verification-firmware pour vérifier le tri, l’annulation, les sélections ambiguës et le CRC32.

- [x] Sélecteur de firmware dans les réglages de machine
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h et Host.h ainsi que src/Emulation/src/Host.cpp pour déclarer les champs de firmware et appliquer les sélections au profil courant.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour afficher la liste triée par rôle, la compatibilité, le chemin, les actions de dossier et la sélection.
  - [x] Créer les catalogues App/emulation-firmware.json dans Constants et Languages à partir des ressources C#.

- [x] Gestion des moteurs dans le SDK
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/EmulatorManager.h avec les contrats des moteurs, installations et versions.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour sélectionner un moteur par configuration, verrouiller les machines enregistrées, rechercher et installer les versions.
  - [x] Créer puis supprimer une sonde dans build/verification-emulator-manager pour couvrir le choix dans un brouillon et son verrouillage après enregistrement.

- [x] Panneau de gestion des moteurs
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour afficher le moteur courant, changer le moteur du brouillon et recharger ses réglages.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour afficher les versions, rechercher et installer avec progression et annulation.
  - [x] Créer les catalogues App/emulation-core.json dans Constants et Languages à partir des ressources C#.

- [x] Contrats et validation du stockage des machines
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/StorageSettings.h et src/Emulation/src/StorageSettings.cpp pour les périphériques, montages, options et règles communes.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h, src/Emulation/src/Host.cpp et src/Emulation/CMakeLists.txt pour exposer les réglages fournis par les modules et valider leur application au brouillon.
  - [x] Créer puis supprimer build/verification-storage-settings pour vérifier les types, les extensions, les doublons et les montages éjectés.

- [x] Cycle de vie du stockage configuré
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour enregistrer les modifications avec le profil et utiliser ses montages au redémarrage.
  - [x] Modifier src/Emulation/src/Host.cpp pour enregistrer les insertions et éjections par lecteur sans écraser les autres réglages du profil.
  - [x] Créer puis supprimer build/verification-session-storage pour vérifier le changement différé et la conservation d’un média éjecté.

- [x] Panneau des lecteurs configurés
  - [x] Créer src/App/Themes/RetroMediaLab/Views/EmulationStoragePanel.h et EmulationStoragePanel.cpp pour afficher les lecteurs et leurs médias et modifier le brouillon.
  - [x] Modifier src/App/CMakeLists.txt et src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour ajouter le panneau et appliquer le brouillon de stockage à l’enregistrement.
  - [x] Créer les catalogues App/emulation-storage.json dans Constants et Languages à partir des ressources C#.

- [x] Création d’images vierges de disquette depuis les réglages
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour créer une image formatée ou un fichier vierge selon le modèle, avec publication sans écrasement.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationStoragePanel.h et EmulationStoragePanel.cpp pour choisir le format, mémoriser le dossier et créer ou créer et insérer l’image.

- [x] Cartes mémoire à taille fixe
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h, StorageSettings.h et src/Emulation/src/Host.cpp pour créer une carte de la taille déclarée, avec initialisation du module et publication sans écrasement.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationStoragePanel.h et EmulationStoragePanel.cpp pour proposer la création et l’association de cartes et contrôler les fichiers existants.
  - [x] Modifier les catalogues App/emulation-storage.json pour reprendre le libellé Common.Create des ressources C#.
  - [x] Créer puis supprimer build/verification-memory-card pour vérifier la taille, le contenu initial, les extensions et les collisions.

- [x] Création de disques durs selon le lecteur émulé
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/StorageSettings.h et src/Emulation/src/StorageSettings.cpp pour déclarer les formats, préparations, tailles et géométries et valider la demande.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h, src/Emulation/src/Host.cpp et src/Emulation/src/MediaPreparation.h et .cpp pour créer les images brutes et connecter les écrivains de MediaEngine et les écrivains spécifiques fournis par le module.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationStoragePanel.h et .cpp pour choisir le format, la préparation, la taille, la géométrie et le fichier, puis créer ou associer le disque.
  - [x] Créer puis supprimer build/verification-hard-disk-storage pour contrôler le disque brut, le jeu .dat/.dsc, les collisions et la géométrie.

- [x] Création des conteneurs de disque dur pris en charge
  - [x] Modifier src/MediaEngine/src/Preparation/Disks.cpp pour créer VHD, VHDX, VDI, VMDK, QCOW2 et CHD avec les écrivains déjà enregistrés.
  - [x] Modifier src/MediaEngine/src/Sources/AtomicImageSetWriter.h et .cpp pour publier les sorties préparées sans écrasement, conserver les fichiers creux et annuler une publication partielle.
  - [x] Créer puis supprimer build/verification-disk-containers pour vérifier la réouverture des formats, les variantes fixes et la protection de la paire VMDK.

- [x] Insertion d’images noires
  - [x] Créer src/VideoPresentation/src/BlackFrameCadence.h et NativeRefreshClock.h et .cpp et modifier src/VideoPresentation/CMakeLists.txt pour synchroniser la présentation aux rafraîchissements de l’écran Windows.
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour conserver la trame rendue, insérer une seule image noire quand le rythme le permet, et enregistrer le réglage avec lecture de l’ancien profil.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App/emulation-video-restoration.json pour contrôler le réglage depuis le dialogue et l’OSD.
  - [x] Créer puis supprimer build/verification-black-frame pour vérifier le rythme, les rafraîchissements sautés et la lecture des deux formes du profil.

- [x] Emplacement et cycle de vie des profils vidéo
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour employer le répertoire de module encodé du C#, relire les profils des versions C++ précédentes, copier et supprimer par identité.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour utiliser Emulation/VideoPresentation et conserver la lecture des profils situés dans le dossier VideoPresentation précédent.
  - [x] Créer puis supprimer build/verification-video-profiles pour vérifier les deux emplacements, la copie, la priorité du nouveau profil et la suppression.

- [x] Gestion des configurations sauvegardées
  - [x] Créer src/App/Themes/RetroMediaLab/Views/EmulationConfigurationsPanel.h et .cpp et modifier src/App/CMakeLists.txt pour le tableau filtrable des configurations, leurs résumés et les commandes de modification et suppression confirmée.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPreferencesDialog.h et .cpp et EmulationPage.cpp pour raccorder les profils vidéo et réutiliser le dialogue de réglages de la machine sélectionnée.
  - [x] Créer les catalogues App/emulation-configurations.json avec les libellés RESX repris de osd-support.

- [x] Contrat des entrées configurées
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/InputSettings.h pour les sources, associations, représentations et périphériques déclarés par les modules.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour décrire, appliquer et enregistrer ces réglages sous leur identité.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationConfigurationsPanel.cpp et les catalogues App/emulation-configurations.json pour afficher les périphériques réellement configurés.

- [x] Associations du clavier et de la souris
  - [x] Créer src/App/Themes/RetroMediaLab/Views/MachineInputBindings.h et MachineInputBindings.cpp et modifier src/App/CMakeLists.txt pour le tableau des actions, la recherche, la capture, la suppression et les valeurs par défaut.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour intégrer les éditeurs aux réglages de machine et valider le brouillon avant enregistrement.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour appliquer les entrées avec les réglages et sauvegarder les associations par le contrat du module.

- [x] Réglages des ports de manette
  - [x] Créer src/App/Themes/RetroMediaLab/Views/MachineControllerPorts.h et MachineControllerPorts.cpp et modifier src/App/CMakeLists.txt pour les ports, catégories, modèles et associations propres au modèle.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour intégrer les réglages des ports au brouillon et au dialogue de machine.
  - [x] Modifier les catalogues App/emulation-controller-feedback.json pour reprendre les libellés et noms de modèles de osd-support.

- [x] Sorties des modèles de manette
  - [x] Créer src/App/Themes/RetroMediaLab/Views/ControllerOutputs.h et ControllerOutputs.cpp et modifier src/App/CMakeLists.txt et EmulationPage.cpp pour partager les sélecteurs de retours physiques.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/MachineControllerPorts.cpp et EmulationPage.cpp pour enregistrer uniquement les sorties du modèle actif.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/InputSettings.h et ControllerFeedback.h et src/Emulation/src/Host.cpp pour configurer les destinations physiques à partir du contrat de port.

- [x] Représentations des périphériques émulés
  - [x] Créer src/App/Themes/RetroMediaLab/Controllers/profiles.json et importer les images référencées depuis osd-support, puis les déclarer dans src/App/CMakeLists.txt.
  - [x] Créer src/App/Themes/RetroMediaLab/Views/ControllerArtwork.h et ControllerArtwork.cpp pour afficher les images et détecter les zones des profils C#.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/MachineInputBindings.h et MachineInputBindings.cpp, MachineControllerPorts.cpp et EmulationPage.cpp pour lancer la capture depuis les zones des manettes et souris et montrer le clavier déclaré.
  - [x] Supprimer le répertoire temporaire build/controller-artwork-export contenant les sources et binaires de l’export de validation.

- [x] Surbrillance des entrées des périphériques
  - [x] Modifier EmulationPhysicalShortcuts.h et MachineInputBindings.h et .cpp dans src/App/Themes/RetroMediaLab/Views pour réutiliser les valeurs des entrées et les associations du brouillon.
  - [x] Modifier ControllerArtwork.h et .cpp, MachineControllerPorts.cpp et EmulationPage.cpp dans src/App/Themes/RetroMediaLab/Views pour mettre en évidence les entrées et arrêter le suivi lorsque la représentation est masquée.

- [x] Accumulation des entrées par image
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/InputFrames.h et src/Emulation/src/InputFrames.cpp et modifier src/Emulation/CMakeLists.txt pour accumulation, consommation et snapshot figé.
  - [x] Créer build/input-frames-check.cpp pour vérifier les quatre mouvements relatifs, la saturation et la conservation des états absolus contre Emulation.dll en Release.
  - [x] Supprimer build/input-frames-check.cpp, build/input-frames-check.obj et build/input-frames-check.exe après la vérification.

- [x] Entrées natives côté SDK
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/EmulationKey.h et InputProcessing.h et src/Emulation/src/InputProcessing.cpp, puis les ajouter dans src/Emulation/CMakeLists.txt pour le décodage des requêtes natives, les valeurs par défaut, la zone morte et la résolution de périphérique.
  - [x] Créer build/native-input-check.cpp pour contrôler la table clavier, les périphériques et les associations avec la DLL Release, puis supprimer cette source et ses produits de compilation.

- [x] Clavier hôte et identifiants du SDK
  - [x] Créer src/App/Themes/RetroMediaLab/Views/EmulationHostKeyboard.h et .cpp et modifier src/App/CMakeLists.txt pour convertir les touches Qt, relâcher la touche physique correspondante et publier les caractères de la disposition.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et src/Emulation/include/RetroMediaLab/Emulation/Sdk.h pour publier les identifiants du SDK et libérer les entrées à la perte de focus.

- [x] Consultation des ports par les adaptateurs
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/InputProcessing.h et src/Emulation/src/InputProcessing.cpp pour exposer les ports et leurs définitions de commandes selon osd-support.

- [x] Événements clavier des cœurs
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/InputProcessing.h et src/Emulation/src/InputProcessing.cpp pour publier les modificateurs, caractères, transitions de touches et amplitudes signées comme sur osd-support.
  - [x] Créer puis supprimer build/native-keyboard-check.cpp et ses binaires après avoir vérifié l’ordre des transitions, la casse et les limites des axes contre Emulation.dll en Release.

- [x] Sélecteur principal fidèle au thème de base C#
  - [x] Modifier EmulationPage.h/.cpp et WorkspaceTabs.cpp sous src/App/Themes/RetroMediaLab/Views pour remplacer les listes et les boutons d’administration par le cadre repliable, les images des marques et machines configurées et l’ouverture de la machine choisie.
  - [x] Ajouter le libellé Emulation.Configuration.Selector issu du C# dans les catalogues App/emulation.json.

- [x] Options d’un module depuis le menu Émulation
  - [x] Modifier MainMenu.h/.cpp, RetroMediaLabLayout.cpp et WorkspaceTabs.h/.cpp pour lister les modules réellement chargés après les préférences et ouvrir leurs options.
  - [x] Modifier EmulationPage.h/.cpp pour choisir une machine dans la liste latérale du dialogue et créer sa première configuration avec le bouton Créer.
  - [x] Modifier EmulationPage.cpp et le stockage commun des brouillons pour conserver les travaux non enregistrés au changement de machine et reprendre la persistance des configurations existantes du C#.
  - [x] Modifier src/App/State/WorkspaceState.h et src/App/Shared/Emulation/EmulationWorkspace.h/.cpp pour stocker les valeurs et associations de chaque brouillon par module et machine et différer l’enregistrement tant que Créer n’a pas été actionné.
- [x] Vérification du brouillon d’émulation
  - [x] Créer dans build/emulation-draft-check un module temporaire et une sonde liée à App.dll pour contrôler que la projection des réglages reste en mémoire avant Créer.
  - [x] Supprimer build/emulation-draft-check et les binaires de la sonde après le contrôle.

- [x] Suivi des options de module
  - [x] Modifier EmulationPage.cpp pour reprendre l’enregistrement immédiat des changements d’une configuration existante à l’intérieur du dialogue, selon osd-support.
  - [x] Modifier EmulationStoragePanel.h/.cpp et EmulationWorkspace.cpp pour signaler les changements de stockage et rapporter la configuration effectivement appliquée.
  - [x] Créer puis supprimer build/emulation-autosave-check et ses binaires pour exercer le menu Émulation, le dialogue et la sauvegarde d’un réglage pendant que le dialogue reste ouvert.

- [x] Périphériques et actions disponibles par port
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/InputProcessing.h et src/Emulation/src/InputProcessing.cpp pour projeter les choix et les commandes en reprenant GetPeripherals et DescribeInput du C#.
  - [x] Créer puis supprimer build/emulation-peripherals-check.cpp et ses binaires après la vérification du bus, de l’ordre USB et de la déduplication.

- [x] Disposition de l’onglet Émulation
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reprendre la hauteur du cadre ouvert, la position des libellés et la présentation de l’accueil du thème Base C#.

Référence C# sur osd-support — EmulationSectionLayoutFunctions.cs et EmulationConfigurationSelector.cs dans le thème Base. Les deux paquets Windows ont été reconstruits. Les 39 PNG sont présents dans Assets/Icons dans les deux paquets.

- [x] Pression des sources d’entrée du SDK
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/EmulationKey.h et src/Emulation/CMakeLists.txt pour exposer les noms des touches à partir de leur déclaration existante.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/InputProcessing.h et src/Emulation/src/InputProcessing.cpp pour résoudre les sources, combinaisons et axes selon EmulationInputMappingFunctions.cs.
  - [x] Créer puis supprimer build/emulation-pressure-check.cpp et ses binaires après vérification des combinaisons, de la zone morte et des manettes ciblées.

  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/EmulationExport.h et modifier Host.h pour partager la déclaration d’export entre Host et EmulationKey.

- [x] Remappage du clavier du SDK
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/InputProcessing.h et src/Emulation/src/InputProcessing.cpp pour porter MapKeyboard avec priorité à la première liaison simple et ajout des touches commandées par les autres entrées.
  - [x] Créer puis supprimer build/emulation-keymap-check.cpp et ses binaires après vérification de la priorité, du remplacement et des entrées combinées.

- [x] Association des directions opposées d’un axe
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/InputProcessing.h et src/Emulation/src/InputProcessing.cpp pour inverser les directions des sticks et des axes numérotés dans une liaison.
  - [x] Modifier src/App/Shared/Controls/MachineInputBindings.cpp pour appliquer l’affectation opposée à la capture lorsque la définition déclare une commande opposée.
  - [x] Créer puis supprimer build/emulation-opposite-check.cpp et ses binaires après vérification de l’inversion.

- [x] Activation des commandes et zones mortes du SDK
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/InputProcessing.h et src/Emulation/src/InputProcessing.cpp pour porter IsMappedInputPressed, MappedInputMagnitude et ApplyDeadZones.
  - [x] Créer puis supprimer build/emulation-mapped-check.cpp et ses binaires après vérification des seuils et des sources.

- [x] Validation et résolution des réglages d’entrée du SDK
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/InputProcessing.h et src/Emulation/src/InputProcessing.cpp pour contrôler les sources, les identifiants d’affectation et la numérotation des ports selon la branche C# osd-support.
  - [x] Créer puis supprimer build/emulation-input-validation-check.cpp et ses binaires après vérification des réglages valides et invalides.

- [x] Format des raccourcis capturés dans les réglages de machine
  - [x] Modifier EmulationHostKeyboard.h/.cpp et EmulationPhysicalShortcuts.h sous src/App/Shared/Emulation pour partager la projection hôte et relire les identifiants SDK dans les contrôles.
  - [x] Modifier EmulationShortcutCapture.h et MachineInputBindings.h/.cpp sous src/App/Shared/Controls ainsi que leurs deux appels dans le thème RetroMediaLab pour enregistrer les touches du SDK et respecter PrefixKeyboardSource.
  - [x] Créer puis supprimer build/emulation-capture-format-check.cpp et ses binaires après vérification du pavé numérique et d’une combinaison.

- [x] Ouverture des supports en cours d’émulation
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour appliquer le filtre du périphérique, proposer la lecture seule aux disquettes et cassettes, et distinguer les supports par type et identifiant.

- [x] Redémarrage demandé par le périphérique émulé
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour reconstruire la machine lors de l’insertion, de l’éjection ou de la réinitialisation matérielle des périphériques concernés.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour rétablir les entrées après changement de machine, reprendre le transport courant et protéger les formats en lecture seule.
  - [x] Créer puis supprimer build/emulation-recreation-check et ses binaires après vérification du cycle avec un module de contrôle.

- [x] Présentation des supports en cours de session
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour appliquer les règles de retrait, les noms, les icônes de média et l’indicateur de présence du thème Base C# dans la vue et son menu plein écran.

- [x] Cartes mémoire en cours de session
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/StorageSettings.h, src/Emulation/src/StorageSettings.cpp et Host.cpp pour valider les cartes et enregistrer leur chemin dans les paramètres du périphérique.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationStoragePanel.h/.cpp et EmulationPage.cpp pour partager les contrôles de sélection, de création et de retrait entre réglages et session.
  - [x] Créer puis supprimer build/emulation-card-check et ses binaires après vérification des chemins, de l’insertion et du retrait.

- [x] Voyants d’activité des supports émulés
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h pour permettre au moteur de signaler les supports actifs sans modifier la table virtuelle existante.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et src/App/Shared/Controls/StatusIndicator.cpp pour afficher l’activité avec la persistance de 140 ms du C# et éviter les redessins à état constant.

- [x] Chargement du libellé du sélecteur de machines
  - [x] Modifier App/emulation.json dans Constants et les 29 catalogues Languages pour ranger Emulation.Configuration.Selector dans la section strings utilisée à la compilation.

- [x] Invariants et textes manquants des catalogues Émulation
  - [x] Modifier les fichiers App/emulation*.json concernés de Constants pour déclarer les références de modèles, normes, unités et noms de moteurs auxquels aucune traduction ne s’applique.
  - [x] Créer sous build/emulation-catalog-completion les catalogues temporaires des textes manquants, puis compléter les entrées absentes sous src/Translations/Catalogs/Languages sans réécrire les traductions existantes.
  - [x] Supprimer build/emulation-catalog-completion après intégration et vérifier les entrées des DLL compilées.

Les 30 DLL du dossier lib/Languages de Release ont été contrôlées. Les 707 clés Émulation sont couvertes dans les 29 langues. 425 entrées ont été vérifiées dans les ressources compilées et 234 invariants sont déclarés dans Constants. Les noms des connexions reprennent les libellés CRT existants. Le nom technique Weave est conservé dans les langues où aucune traduction appropriée n’a été retenue.

- [x] Filtres de mise à l’échelle Sharp bilinear, Bicubic, Jinc2 et Lanczos
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h, src/VideoPresentation/src/Presentation.cpp et SoftwareProcessing.cpp et créer src/VideoPresentation/src/SamplingFilters.h pour porter les quatre filtres logiciels de osd-support et lire leurs identifiants dans les profils.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les fichiers App/emulation.json des catalogues de src/Translations pour utiliser les mêmes choix dans le dialogue et l’OSD.
  - [x] Créer puis supprimer build/emulation-sampling-check après vérification des quatre filtres, du redimensionnement et des profils.

Release vérifiée avec une trame synthétique. Les quatre résultats sont distincts, suivent le redimensionnement et retrouvent leur identifiant après sauvegarde. Les libellés reprennent EmulationVideo.resx de osd-support. Treize filtres restent à porter.

- [x] Filtres ScaleNx, EPX, 2xSaI, Super 2xSaI et Super Eagle
  - [x] Modifier Presentation.h, Presentation.cpp et SamplingFilters.h sous src/VideoPresentation et créer src/VideoPresentation/src/PixelArtSampling.h pour porter les règles et identifiants de osd-support.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et src/Translations/Catalogs/Constants/App/emulation.json pour exposer les cinq noms invariants dans le dialogue et l’OSD.
  - [x] Créer puis supprimer build/emulation-pixel-art-check après vérification du rendu, des profils et du changement de taille.

Release installée et vérifiée. Les valeurs aux coins ont été comparées aux règles Scale2x, Scale3x et aux coefficients des variantes SaI. Les cinq profils sont sauvegardés et relus, et les résultats suivent la taille du rendu.

- [x] Filtres xBR, xBRZ, HQx, HQ2x, HQ3x, HQ4x, ScaleFX et SABR
  - [x] Créer src/VideoPresentation/src/EdgeAwareSampling.h et modifier SamplingFilters.h et SoftwareProcessing.cpp pour porter les interpolations et mesures de couleur du C#.
  - [x] Modifier Presentation.h et Presentation.cpp sous src/VideoPresentation, EmulationPage.cpp sous src/App/Themes/RetroMediaLab/Views et Constants/App/emulation.json sous src/Translations/Catalogs pour exposer et sauvegarder les huit filtres.
  - [x] Créer puis supprimer build/emulation-edge-sampling-check après vérification des sorties et des profils.
  - [x] Créer src/VideoPresentation/THIRD-PARTY-NOTICES.md à partir des trois notices MIT de osd-support et modifier src/VideoPresentation/CMakeLists.txt pour les distribuer.

Release vérifiée avec une trame synthétique. Les huit résultats sont distincts, les profils sont relus et le rendu suit le redimensionnement. HQx reste lisible dans les profils et ne figure dans les choix que si le profil actif l’utilise. Les dix-neuf algorithmes logiciels sont désormais disponibles. Le traitement matériel des filtres reste à vérifier face au C#.

- [x] Traitement asynchrone des trames pendant l’émulation
  - [x] Créer src/VideoPresentation/src/SoftwareFrameWorker.h et modifier Presentation.h et Presentation.cpp pour limiter l’attente à la dernière trame et invalider les résultats périmés.
  - [x] Modifier src/VideoPresentation/src/SoftwareProcessing.h et SoftwareProcessing.cpp pour conserver le chemin direct des trames sans effets.
  - [x] Créer puis supprimer build/emulation-frame-worker-check après vérification de la file bornée, des changements de profil et de taille, et de la fermeture.

Release installée et vérifiée hors écran. La dernière trame d’une rafale est affichée après redimensionnement. Les changements de profil, l’effacement, le masquage et la destruction écartent les résultats périmés. Les surfaces Qt et matérielles consomment l’image préparée. Le portage des filtres vers les shaders des moteurs matériels reste ouvert.

- [x] Mise à l’échelle des trames OpenGL
  - [x] Créer src/VideoPresentation/src/OpenGlSamplingShader.h à partir des fonctions GLSL de osd-support et modifier OpenGlSurface.h et OpenGlSurface.cpp pour compiler le shader et utiliser la texture de la trame.
  - [x] Modifier Presentation.h et Presentation.cpp sous src/VideoPresentation pour utiliser la mise à l’échelle OpenGL, conserver le traitement logiciel des combinaisons d’effets et préserver la capture d’écran.
  - [x] Créer puis supprimer build/emulation-opengl-sampling-check après vérification dans un contexte OpenGL et contrôle du paquet Release.

La Release installée a été vérifiée sur le contexte OpenGL Windows à 150 %. Les dix-sept filtres avancés ont produit une image. Le pixel bicubique capturé correspond au pixel présenté et le chemin logiciel produit une valeur distincte sur la même trame synthétique. Les profils combinant un filtre et un autre effet continuent d’utiliser la chaîne logicielle.

- [x] Mise à l’échelle des trames Vulkan
  - [x] Créer src/VideoPresentation/shaders/sampling.vert et sampling.frag avec les filtres Veldrid de osd-support et modifier src/VideoPresentation/CMakeLists.txt pour compiler le shader.
  - [x] Modifier VulkanSurface.h et VulkanSurface.cpp sous src/VideoPresentation/src pour présenter les trames et l’OSD avec QRhi et capturer la zone vidéo.
  - [x] Modifier Presentation.h et Presentation.cpp sous src/VideoPresentation pour raccorder le rendu Vulkan au profil et gérer le repli logiciel.
  - [x] Créer puis supprimer build/emulation-vulkan-sampling-check après vérification avec le moteur Vulkan.

Release contrôlée avec le moteur Vulkan sous Windows à 150 %. Les dix-sept filtres avancés produisent une image. Le pixel bicubique capturé correspond au pixel affiché et diffère de celui du moteur logiciel. L’OSD modifie l’affichage tout en restant hors de la capture vidéo. Les shaders sont aussi compilés en HLSL et le raccordement à Direct3D 11 reste ouvert.

- [x] Mise à l’échelle des trames Direct3D 11
  - [x] Créer src/VideoPresentation/src/HardwareSamplingShader.h et modifier VulkanSurface.cpp pour partager les chemins et paramètres des shaders Veldrid.
  - [x] Modifier Direct3DSurface.h et Direct3DSurface.cpp sous src/VideoPresentation/src pour charger les variantes HLSL, présenter les filtres et capturer la trame sans OSD.
  - [x] Modifier src/VideoPresentation/src/Presentation.cpp pour choisir le filtre Direct3D avec le même repli que les autres moteurs.
  - [x] Créer puis supprimer build/emulation-direct3d-sampling-check et les deux fichiers temporaires build/video-sampling-vert.hlsl et build/video-sampling-frag.hlsl après contrôle du moteur.

Release vérifiée avec la chaîne de présentation Direct3D 11 sous Windows à 150 %. Les dix-sept filtres avancés ont produit une capture. Le pixel bicubique Direct3D correspond à la valeur Vulkan de la même trame et diffère de la valeur logicielle. Le contrôle a aussi vérifié le redimensionnement et l’absence de l’OSD dans la capture.

- [x] Corrections d’image sur les moteurs matériels
  - [x] Créer src/VideoPresentation/shaders/image-adjustments.glsl depuis osd-support et modifier sampling.vert, sampling.frag, OpenGlSamplingShader.h et src/VideoPresentation/CMakeLists.txt pour partager les fonctions et les charger dans les deux variantes du shader.
  - [x] Modifier SoftwareProcessing.h et SoftwareProcessing.cpp ainsi que HardwareSamplingShader.h sous src/VideoPresentation/src pour partager les paramètres normalisés.
  - [x] Modifier Presentation.h/.cpp et OpenGlSurface.h/.cpp, VulkanSurface.h/.cpp et Direct3DSurface.h/.cpp sous src/VideoPresentation pour raccorder les réglages colorimétriques et la netteté aux trames et aux captures.
  - [x] Créer puis supprimer build/emulation-gpu-adjustments-check après vérification des corrections combinées, de leur application à la capture et de la préservation du rendu de base.

Contrôle Windows Release avec OpenGL, Vulkan et Direct3D 11. Le bicubique, les corrections de couleur et la netteté se combinent à la capture. La réinitialisation restitue exactement la capture de base pour les trois moteurs. Le réglage de luminosité fonctionne aussi en plus proche voisin, y compris après redimensionnement.

- [x] Sélecteur de l’onglet Émulation et images externes
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour centrer le texte d’accueil et reprendre la disposition du bouton Ouvrir du C#.
  - [x] Ajouter sous src/App/Assets/Emulation les PNG de marques et de machines de osd-support, dans la même arborescence que la sortie C#.
  - [x] Compléter les PNG applicatifs de src/App/Assets/Controllers, Media et Icons à partir de osd-support, puis vérifier leur copie dans Assets après compilation.

Référence C# sur `origin/osd-support` (`15444aeec`), thème Base. Debug et Release contiennent les 495 PNG d’App à l’identique, dont 229 PNG de marques et de machines sous `Assets/Emulation`. Dans le paquet Release, l’onglet affiche le cadre repliable, les deux rangées et l’accueil centré. Le dossier Modules est vide dans ce paquet de vérification.

- [x] Préparation des disques durs AmigaDOS
  - [x] Modifier src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/AmigaDos.h et src/MediaFileSystems/src/FileSystems/Amiga/Dos/Writer.cpp et Constants.h pour produire les secteurs d’amorçage, racine, bitmaps et extensions suivant AmigaDosVolumeFormatter sur osd-support.
  - [x] Modifier src/MediaFileSystems/src/FileSystems/Amiga/Dos/Reader.cpp pour compter aussi les bitmaps d’extension, avec contrôle des pointeurs.
  - [x] Modifier src/MediaEngine/src/Preparation/Disks.cpp pour transmettre les régions AmigaDOS aux écrivains de conteneurs existants et publier le résultat atomiquement.
  - [x] Créer puis supprimer build/emulation-amigados-volume-check après avoir vérifié la relecture des variantes et la publication des images.

- [x] Création d’images Amiga RDB
  - [x] Créer src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Rdb.h et src/MediaFileSystems/src/Partitioning/Rdb.cpp pour valider les partitions et écrire les blocs RDSK et PART. Raccorder le fichier à src/MediaFileSystems/CMakeLists.txt.
  - [x] Modifier src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaPreparation.h et src/MediaEngine/src/Preparation/Disks.cpp pour publier les préparations RDB OFS et FFS suivant AmigaRdbPartitionWriter.
  - [x] Créer puis supprimer build/emulation-rdb-check et les images temporaires de validation.

- [x] Lecture des partitions Amiga RDB
  - [x] Modifier src/MediaFileSystems/src/Partitioning/Rdb.cpp, Volumes.cpp, include/RetroMediaLab/MediaFileSystems/Volumes.h, VolumeConstants.h et Rdb.h pour détecter RDSK et parcourir les blocs PART bornés.
  - [x] Modifier src/MediaFileSystems/src/Exploration/Volumes/Detectors.cpp et src/MediaFileSystems/src/Exploration/Composition/Catalog.cpp et créer AmigaDosDocument.cpp pour lire les volumes AmigaDOS des représentations en blocs. Ajouter le fichier à src/MediaFileSystems/CMakeLists.txt.
  - [x] Créer puis supprimer build/emulation-rdb-explorer-check, les binaires et les images temporaires, après vérification depuis l’entrée d’exploration.

- [x] Préparation FAT16 Atari
  - [x] Créer src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/AtariHardDisk.h et src/MediaFileSystems/src/FileSystems/Fat/AtariHardDisk.cpp pour préparer la table AHDI, le BPB FAT16, les deux FAT et le répertoire racine. Ajouter le fichier à src/MediaFileSystems/CMakeLists.txt.
  - [x] Modifier src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaPreparation.h et src/MediaEngine/src/Preparation/Disks.cpp pour acheminer les préparations AHDI et IDE avec inversion des mots vers les écrivains existants.
  - [x] Créer puis supprimer build/emulation-atari-hard-disk-check, les binaires et les images temporaires de validation.

- [x] Ouverture des disques Atari AHDI et IDE
  - [x] Modifier src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/AtariHardDisk.h, src/MediaFileSystems/src/FileSystems/Fat/AtariHardDisk.cpp, src/MediaFileSystems/src/Partitioning/Volumes.cpp, src/MediaFileSystems/src/Exploration/Volumes/Detectors.cpp et src/MediaEngine/src/Formats/HardDisk/Raw/Reader.cpp pour reconnaître les partitions AHDI, leurs extensions XGM et la variante IDE.
  - [x] Créer src/MediaFileSystems/src/Exploration/Composition/BlockVolume.h à partir d’AmigaDosDocument.cpp et adapter le lecteur FAT dans src/MediaFileSystems/src/FileSystems/Fat/Fat12/{Fat12Internal.h,Tables.cpp,Reader.cpp} aux chaînes FAT16. Créer src/MediaFileSystems/src/Exploration/Composition/AtariFatDocument.cpp et le raccorder dans src/MediaFileSystems/src/Exploration/MediaExplorer.cpp et src/MediaFileSystems/CMakeLists.txt.
  - [x] Créer puis supprimer build/emulation-atari-explorer-check et ses images temporaires après vérification du catalogue et de la lecture des fichiers.

- [x] Registre réseau commun
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/Networks.h et src/Emulation/src/Networks.cpp avec les contrats, les clés de libellé et le registre de présence et de compatibilité de osd-support. Raccorder ces fichiers dans src/Emulation/CMakeLists.txt.
  - [x] Modifier src/Emulation/src/Host.cpp pour publier les descriptions réseau quand les configurations sont chargées, enregistrées ou supprimées.
  - [x] Créer puis supprimer build/emulation-networks-check après validation des publications, des incompatibilités et du retrait automatique d’une interface.
  - [x] Créer src/Translations/Catalogs/Constants/App/emulation-network.json et les fichiers homologues dans Languages avec les textes réseau de EmulationWelcome.resx.
  - [x] Créer src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-network-errors.json pour les diagnostics du registre et retirer les fichiers de génération temporaires dans build/emulation-networks-check.

- [x] Affichage réseau des préférences d’Émulation
  - [x] Créer src/App/Themes/RetroMediaLab/Views/EmulationNetworkPanel.h et EmulationNetworkPanel.cpp pour le tableau de suivi et sa mise à jour depuis le registre.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPreferencesDialog.cpp et src/App/CMakeLists.txt pour raccorder le panneau Réseau aux préférences.
  - [x] Créer puis supprimer build/emulation-network-panel-check après vérification des cellules, de la traduction et des mises à jour du registre.

- [x] Dossiers généraux d’Émulation
  - [x] Créer src/Infrastructure/State/EmulationFolders.h et modifier PreferencesStore.h, PreferencesStore.cpp, src/App/State/WorkspaceState.h, WorkspaceState.cpp et src/App/Bootstrap/Application.cpp pour lire, valider et enregistrer les trois chemins dans preferences.json.
  - [x] Modifier src/App/Shared/Emulation/EmulationWorkspace.h et EmulationWorkspace.cpp, src/Emulation/include/RetroMediaLab/Emulation/Host.h et Sdk.h, src/Emulation/src/Host.cpp et les vues EmulationPage.cpp et EmulationStoragePanel.cpp pour utiliser les dossiers résolus dans les services, captures, états et sélecteurs existants.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPreferencesDialog.cpp pour afficher les trois champs Général et leurs boutons de parcours. Créer src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-folders.json à partir des libellés du C#.
  - [x] Créer puis supprimer build/emulation-folders-check après vérification de la persistance et des dossiers appliqués.

- [x] Mise à jour du sélecteur Émulation dans Debug
  - [x] Régénérer build/Debug/Retro Media Lab/lib/Themes/RetroMediaLab.dll par le build et l’installation Debug avec le sélecteur à vignettes de osd-support.

- [x] Enregistrement des raccourcis d’Émulation
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPreferencesDialog.cpp pour enregistrer les modifications valides pendant l’édition et placer Restaurer et Effacer les conflits à côté de la recherche, selon osd-support.
  - [x] Modifier src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-shortcuts.json pour reprendre le libellé Effacer les conflits des RESX C#.
  - [x] Régénérer la DLL du thème et les catalogues installés sous build/Release/Retro Media Lab.
  - [x] Créer puis supprimer build/emulation-shortcuts-check pour le contrôle ciblé de l’enregistrement et des conflits.

- [x] Réorganisation des onglets des machines
  - [x] Créer src/App/Themes/RetroMediaLab/Views/EmulationMachineTabs.h avec le glissement des onglets de machines et leur repère d’insertion, en conservant Accueil en première position.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour utiliser cette barre dans Émulation.
  - [x] Régénérer build/Release/Retro Media Lab/lib/Themes/RetroMediaLab.dll.
  - [x] Créer puis supprimer build/emulation-machine-tabs-check après le contrôle de l’ordre, de la sélection et de la protection d’Accueil.

- [x] Commandes de cycle de vie dans la barre de session
  - [x] Créer src/App/Themes/RetroMediaLab/Views/EmulationCommandIcons.h avec les glyphes et couleurs du C# et les icônes Qt de secours sur les autres plateformes.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour regrouper Alimentation, Pause, Redémarrage logiciel et Redémarrage matériel. Utiliser la même action pour le bouton et son raccourci.
  - [x] Régénérer build/Release/Retro Media Lab/lib/Themes/RetroMediaLab.dll.

- [x] Groupes Sauvegarde rapide et Affichage de la barre de session
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationCommandIcons.h avec les glyphes Sauvegarde, Chargement, Capture, Plein écran et Paramètres des RESX.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour grouper ces commandes selon MachineCommandBar.cs, conserver les actions existantes et reprendre les états de disponibilité et la couleur de reprise.
  - [x] Régénérer build/Release/Retro Media Lab/lib/Themes/RetroMediaLab.dll.

- [x] Audio, manette et souris dans la barre de session
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationCommandIcons.h pour reprendre les glyphes audio, muet, manette et souris, ainsi que les couleurs de statut du C#.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour regrouper la commande audio et les indicateurs d’entrée, actualiser leur état et conserver la commande audio de l’OSD.
  - [x] Modifier src/Translations/Catalogs/{Constants,Languages/*}/App/emulation.json pour reprendre les états accessibles de EmulationValue.resx.
  - [x] Régénérer les DLL de thème et de langue sous build/Release/Retro Media Lab.

- [x] Bascule manette et souris déclarée par la machine
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/{Sdk,Host}.h et src/Emulation/src/Host.cpp pour porter la capacité facultative du C#, son état et sa commande selon l’état de la machine.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour présenter la bascule du C# à côté de l’audio et des indicateurs.
  - [x] Modifier src/Translations/Catalogs/{Constants,Languages/*}/App/emulation.json avec le libellé de EmulationController.resx.
  - [x] Régénérer les DLL de thème, d’Émulation et de langue sous build/Release/Retro Media Lab.
  - [x] Créer puis supprimer build/emulation-pointer-switch-check après vérification avec un module temporaire.

- [x] Vérification de la barre avec une machine chargée
  - [x] Créer puis supprimer build/emulation-command-bar-check avec un module temporaire et une sonde de la vue du thème. Vérifier les clics de cycle de vie, les sauvegardes rapides, l’audio et la bascule facultative.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour actualiser les commandes de sauvegarde rapide immédiatement après une action de session. Régénérer build/Release/Retro Media Lab/lib/Themes/RetroMediaLab.dll.

- [x] Raccourcis dans les infobulles de session
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reprendre l’association entre commandes et raccourcis de MachineCommandBar.cs, et actualiser les infobulles des sessions ouvertes lorsque les réglages changent.
  - [x] Régénérer build/Release/Retro Media Lab/lib/Themes/RetroMediaLab.dll.
  - [x] Créer puis supprimer build/emulation-command-bar-check après vérification des raccourcis par défaut, personnalisés et effacés dans la vue du thème.

- [x] Mesure de cadence et de vitesse d’émulation
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/ExecutionRate.h et src/Emulation/src/ExecutionRate.cpp. Raccorder la mesure des fenêtres actives et l’horloge facultative dans src/Emulation/CMakeLists.txt.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour afficher la cadence constatée et l’horloge effective dans les statistiques de session.
  - [x] Modifier src/Translations/Catalogs/Constants/App/emulation.json avec le format invariant d’horloge de Status.resx.
  - [x] Régénérer les DLL de thème, d’Émulation et de langue sous build/Release/Retro Media Lab.
  - [x] Créer puis supprimer build/emulation-execution-rate-check pour vérifier les mesures, les doublons et les ruptures de séquence.

- [x] Cadence des frames et redémarrage logiciel communs
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/MachineExecution.h et src/Emulation/src/MachineExecution.cpp avec un minuteur possédé et annulable, la cadence absolue bornée et le relâchement des touches de redémarrage logiciel.
  - [x] Modifier src/Emulation/CMakeLists.txt pour raccorder les services au SDK Émulation.
  - [x] Régénérer build/Release/Retro Media Lab/lib/Emulation.dll.
  - [x] Créer puis supprimer build/emulation-machine-execution-check après le contrôle des échéances, de l’annulation et du relâchement des touches.

- [x] File et boucles d’exécution des machines
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/MachineExecution.h et src/Emulation/src/MachineExecution.cpp pour reprendre EmulationMachineCommand.cs et les deux boucles de EmulationFrameTimer.cs.
  - [x] Régénérer build/Release/Retro Media Lab/lib/Emulation.dll.
  - [x] Créer puis supprimer build/emulation-machine-command-check après contrôle du thread d’exécution, de l’ordre, de la pause, des erreurs et de l’annulation.

- [x] Publication des sorties des machines
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/MachineExecution.h pour publier une seule fois chaque nouvelle séquence vidéo puis transmettre les blocs audio dans l’ordre, comme EmulationFrameOutputFunctions.cs.
  - [x] Créer puis supprimer build/emulation-machine-output-check après vérification de l’ordre, de la déduplication et de la propagation d’une erreur de publication.

- [x] Arrêt et nettoyage communs des cœurs externes
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/MachineExecution.h et src/Emulation/src/MachineExecution.cpp pour reprendre RunCore, Cleanup, DisposeMachineAsync et TryDisposeCore de ExternalCoreLifecycleFunctions.cs avec les appels C++ synchrones.
  - [x] Régénérer build/Release/Retro Media Lab/lib/Emulation.dll.
  - [x] Créer puis supprimer build/emulation-core-lifecycle-check après contrôle des erreurs et de l’ordre de libération.

- [x] Sauvegarde et restauration des cœurs externes
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/MachineExecution.h et src/Emulation/src/MachineExecution.cpp pour reprendre les contrôles de taille et la sauvegarde et restauration natives de ExternalCoreLifecycleFunctions.cs.
  - [x] Régénérer build/Release/Retro Media Lab/lib/Emulation.dll.
  - [x] Créer puis supprimer build/emulation-core-state-check après vérification du transfert des octets, des bornes et des erreurs natives.

- [x] Activité des lecteurs remontée par les cœurs
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/StorageSettings.h et src/Emulation/src/StorageSettings.cpp pour reprendre la conversion des identifiants de voyants de EmulationMediaRules.cs.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostedCoreSession.h et src/Emulation/src/HostedCoreSession.cpp pour exposer cette conversion sur les états du cœur hébergé.
  - [x] Régénérer build/Release/Retro Media Lab/lib/Emulation.dll.
  - [x] Créer puis supprimer build/emulation-media-leds-check après contrôle des voyants définis, manquants et optiques.

- [x] Restauration de l’image par les moteurs matériels
  - [x] Créer src/VideoPresentation/shaders/image-restoration.glsl depuis VideoImageRestorationShader.cs et modifier CMakeLists.txt, image-adjustments.glsl et sampling.frag pour partager le débandeur et la récupération des détails.
  - [x] Modifier HardwareSamplingShader.h, OpenGlSamplingShader.h et les interfaces et implémentations OpenGlSurface, VulkanSurface, Direct3DSurface et Presentation pour appliquer les paramètres à l’affichage et aux captures.
  - [x] Régénérer build/Release/Retro Media Lab/lib/VideoPresentation.dll.
  - [x] Créer puis supprimer build/emulation-gpu-restoration-check pour vérifier le débandeur et la récupération des détails dans les captures des trois moteurs matériels et du moteur logiciel.

- [x] Changement à chaud entre OpenGL et Vulkan
  - [x] Modifier src/VideoPresentation/src/VulkanSurface.h et VulkanSurface.cpp pour isoler le moteur Vulkan de la chaîne de composition de la fenêtre principale après vérification du comportement des contextes Qt.
  - [x] Modifier src/VideoPresentation/src/Presentation.cpp pour gérer le conteneur et préserver les trames, le redimensionnement et les captures.
  - [x] Régénérer build/Release/Retro Media Lab/lib/VideoPresentation.dll.
  - [x] Créer puis supprimer build/emulation-renderer-switch-check pour vérifier les deux sens de bascule pendant une session et contrôler les pixels capturés.
  - [x] Modifier src/VideoPresentation/src/Presentation.cpp pour réutiliser la surface Vulkan masquée à la seconde bascule et libérer sa surface à la fermeture du lecteur.

- [x] Commandes de la machine dans le menu plein écran
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour créer l’entrée Commandes, regrouper les ports, le clavier et la souris et afficher les affectations en lecture seule comme MachineController.OsdControls.cs.
  - [x] Modifier les fichiers src/Translations/Catalogs/Constants/App/emulation-controller-feedback.json et src/Translations/Catalogs/Languages/*/App/emulation-controller-feedback.json avec les libellés C# des commandes et des affectations vides.
  - [x] Régénérer le thème et les DLL de langues sous build/Release/Retro Media Lab.

- [x] Présentation des affectations dans le menu Contrôles
  - [x] Modifier src/App/Shared/Controls/MachineInputBindings.h et MachineInputBindings.cpp pour partager les libellés issus de InputBindingRow.cs entre le dialogue et le menu plein écran.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour utiliser la présentation commune des affectations.
  - [x] Modifier les catalogues App/emulation-controller-feedback.json de Constants et des langues avec les noms des commandes clavier, souris et manette des RESX d’origine.
  - [x] Régénérer App.dll, la DLL du thème et les catalogues dans build/Release/Retro Media Lab.
  - [x] Créer puis supprimer build/emulation-binding-label-check après vérification des combinaisons, des directions et des valeurs inconnues.

- [x] Navigation et contenu du menu plein écran
  - [x] Modifier src/VideoPresentation/src/FullscreenOsd.cpp et src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h pour permettre la navigation entre commandes du menu et actualiser son sous-titre.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reprendre la liste et la visibilité des rubriques du C#, et afficher le nom du support inséré choisi par le C#.
  - [x] Régénérer VideoPresentation.dll et la DLL du thème dans build/Release/Retro Media Lab.
  - [x] Créer puis supprimer build/emulation-osd-navigation-check après vérification de la navigation et de la mise à jour du titre.

- [x] Surimpression plein écran au-dessus des moteurs natifs
  - [x] Modifier FullscreenOsd.h et FullscreenOsd.cpp sous src/VideoPresentation pour reprendre la fenêtre transparente possédée par le plein écran du C# et suivre la géométrie de son propriétaire.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour transférer les raccourcis vers la fenêtre OSD et les restituer avec le focus en la fermant.
  - [x] Régénérer VideoPresentation.dll et la DLL du thème sous build/Release/Retro Media Lab.
  - [x] Créer puis supprimer build/emulation-osd-overlay-check après contrôle des fenêtres natives, de leur position et de la navigation clavier.

- [x] Raccourcis affichés au pied du menu plein écran
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour fournir les raccourcis actifs, les variantes et les libellés de navigation de osd-support, y compris après une modification des préférences.
  - [x] Modifier FullscreenOsd.h et FullscreenOsd.cpp sous src/VideoPresentation pour afficher les raccourcis et les indications dans les deux rangées du C#.
  - [x] Régénérer VideoPresentation.dll et la DLL du thème sous build/Release/Retro Media Lab.
  - [x] Créer puis supprimer build/emulation-osd-footer-check après vérification du rendu des raccourcis et de leur remplacement.

- [x] Précompilation des shaders Direct3D 11
  - [x] Modifier src/VideoPresentation/CMakeLists.txt pour embarquer le DXBC lorsque fxc est disponible et conserver le shader source pour les configurations sans fxc.
  - [x] Modifier src/VideoPresentation/src/Direct3DSurface.cpp pour charger les shaders précompilés et leurs liaisons sans compilation à l’ouverture de la vue.
  - [x] Régénérer build/Release/Retro Media Lab/lib/VideoPresentation.dll.
  - [x] Créer puis supprimer build/emulation-d3d-precompiled-check après contrôle de la présentation et du chemin matériel de capture.

- [x] Indicateur de préparation des shaders vidéo
  - [x] Modifier les fichiers App/emulation-video-processing.json de Constants et de chaque langue pour reprendre Emulation.Video.Shader.Loading du C# osd-support.
  - [x] Modifier les surfaces vidéo et leur interface pour publier le début et la fin des préparations natives depuis les chemins de création des shaders.
  - [x] Créer puis supprimer build/emulation-shader-loading-check après contrôle des notifications OpenGL, Vulkan et Direct3D 11.
  - [x] Modifier EmulationPage.cpp pour présenter le panneau de préparation centré devant la vidéo et le menu plein écran.
  - [x] Modifier EmulationPage.cpp pour relayer la préparation aux réglages vidéo de la machine active.
  - [x] Régénérer la DLL du thème, VideoPresentation et lib/Languages dans le build Windows Release.
  - [x] Créer puis supprimer build/emulation-shader-badge-check après vérification des transitions, de la géométrie et des réglages dans une session Release.
  - [x] Modifier src/VideoPresentation/src/Direct3DSurface.cpp pour préparer les programmes Direct3D 11 sur un fil de travail et reprendre ensuite la présentation sur le fil graphique.
  - [x] Régénérer build/Release/Retro Media Lab/lib/VideoPresentation.dll.
  - [x] Créer puis supprimer build/emulation-d3d-worker-check après contrôle de la réponse de la fenêtre et du passage au rendu Direct3D 11.
  - [x] Modifier src/VideoPresentation/src/OpenGlSurface.h et .cpp pour préparer le programme dans un contexte partagé sur un fil de travail lorsque la plateforme le permet.
  - [x] Régénérer build/Release/Retro Media Lab/lib/VideoPresentation.dll après la modification OpenGL.
  - [x] Créer puis supprimer build/emulation-gl-worker-check après contrôle du rendu partagé, de la réactivité et du changement de moteur.
  - [x] Créer puis supprimer build/emulation-gl-badge-check après contrôle de la peinture du panneau pendant la compilation OpenGL.
  - [x] Modifier src/VideoPresentation/src/VulkanSurface.cpp pour réutiliser le pipeline entre cibles compatibles d’un même périphérique et garder les QShader déjà chargés.
  - [x] Régénérer build/Release/Retro Media Lab/lib/VideoPresentation.dll après la modification Vulkan.
  - [x] Créer puis supprimer build/emulation-vulkan-pipeline-check après contrôle de la recréation de cible, de l’absence de reliaison superflue et de la capture Vulkan.
  - [x] Modifier la préparation de la première pipeline Vulkan pour traiter le coût du pilote hors du fil graphique lorsque l’API le permet.
    - [x] Créer puis supprimer build/emulation-vulkan-cache-check pour vérifier la compilation d’un pipeline sur une instance Vulkan hors écran et mesurer sa réutilisation.
    - [x] Ajouter les en-têtes C et les licences Vulkan-Headers Khronos dans third_party/vulkan-headers et supprimer le dossier temporaire build/emulation-vulkan-headers.
    - [x] Modifier src/VideoPresentation/CMakeLists.txt pour fournir les en-têtes à Qt et installer les notices de licence.
    - [x] Modifier src/VideoPresentation/src/VulkanSurface.h et .cpp pour générer et réutiliser le cache en mémoire par périphérique et format de cible.
    - [x] Régénérer build/Release/Retro Media Lab/lib/VideoPresentation.dll.
    - [x] Créer puis supprimer build/emulation-vulkan-worker-check après contrôle du chargement asynchrone, de la reprise du rendu et de la capture.
    - [x] Ajouter puis retirer les traces temporaires de VulkanSurface.cpp et mesurer le passage du cache et la plus longue interruption dans build/emulation-vulkan-worker-check.

- [x] Vérification du sélecteur principal dans les paquets Windows
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reprendre la marge et le libellé d’accessibilité du bouton de repli du thème Base.
  - [x] Régénérer lib/Themes/RetroMediaLab.dll dans les paquets Windows Debug et Release.
  - [x] Créer puis supprimer build/emulation-selector-check après contrôle de l’accueil et du cadre repliable dans le thème Release sans module installé.

Le thème chargé depuis lib/Themes dans le paquet Release a été vérifié sans module, avec ouverture, repli et réouverture du cadre sans listes déroulantes. Les deux paquets contiennent les 495 PNG de src/App/Assets, dont 39 dans Assets/Icons.

- [x] Rétablir le chemin Vulkan vérifié avant le test de préparation hors écran
  - [x] Modifier src/VideoPresentation/src/VulkanSurface.h et .cpp pour retirer la préparation expérimentale après le blocage observé lors d’un changement de moteur.
  - [x] Modifier src/VideoPresentation/CMakeLists.txt et THIRD-PARTY-NOTICES.md pour retirer la dépendance Vulkan-Headers ajoutée pour cet essai, et supprimer third_party/vulkan-headers.
  - [x] Régénérer VideoPresentation.dll dans les paquets Windows Release et Debug.

- [x] Diagnostiquer le changement de moteur Vulkan dans une session ouverte
  - [x] Modifier temporairement src/VideoPresentation/src/VulkanSurface.cpp et Presentation.cpp pour localiser le blocage, puis supprimer les traces.
  - [x] Créer puis supprimer les sondes build/emulation-vulkan-switch-* après contrôle du changement de moteur dans le thème Release.
  - [x] Modifier Presentation.cpp, VulkanSurface.cpp et VulkanSurface.h pour séparer la fenêtre de composition Vulkan de la hiérarchie QWidget et la rattacher à la fenêtre hôte native.
  - [x] Régénérer VideoPresentation.dll dans le paquet Windows Release après vérification du basculement et de la capture.

- [x] Diagnostics natifs de démarrage des cœurs
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/CoreDiagnostics.h et src/Emulation/src/CoreDiagnostics.cpp pour mémoriser le premier échec, différer son exception et joindre les diagnostics à la cause initiale comme sur osd-support.
  - [x] Modifier src/Emulation/CMakeLists.txt pour compiler les fonctions dans Emulation.dll.
  - [x] Créer puis supprimer build/emulation-core-diagnostics-check après contrôle de la publication concurrente, de l’exception différée et des diagnostics ordonnés.
  - [x] Régénérer Emulation.dll dans le paquet Windows Release.

- [x] Chaînes et contenu des cœurs natifs
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/NativeCoreContent.h et src/Emulation/src/NativeCoreContent.cpp pour les chaînes terminées par zéro, le stockage par clé et le descripteur de chargement de osd-support.
  - [x] Modifier src/Emulation/CMakeLists.txt pour exporter les propriétaires depuis Emulation.dll.
  - [x] Créer puis supprimer build/emulation-native-content-check après contrôle des octets, des pointeurs empruntés, du chargement par chemin et des erreurs.
  - [x] Régénérer Emulation.dll dans le paquet Windows Release.

- [x] Chargement et inspection des bibliothèques de cœur
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/NativeCoreLibrary.h et src/Emulation/src/NativeCoreLibrary.cpp pour les exports, la validation ABI et la copie des métadonnées comme sur osd-support.
  - [x] Modifier src/Emulation/CMakeLists.txt pour compiler le chargeur dans Emulation.dll.
  - [x] Créer puis supprimer build/emulation-native-library-check avec une bibliothèque temporaire pour contrôler le chemin, les exports, la version, les métadonnées et la libération.
  - [x] Régénérer Emulation.dll dans le paquet Windows Release.

- [x] Réception audio et vidéo des cœurs natifs
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/NativeCoreFrames.h et src/Emulation/src/NativeCoreFrames.cpp pour copier les pixels et les échantillons selon osd-support, borner la file et mémoriser la première erreur du callback.
  - [x] Modifier src/Emulation/CMakeLists.txt pour compiler ces propriétaires communs.
  - [x] Créer puis supprimer build/emulation-native-frames-check après contrôle des copies, des limites, des compteurs et de la répétition des trames.
  - [x] Régénérer Emulation.dll dans le paquet Windows Release.

- [x] Commandes d’environnement audio, vidéo et entrées des cœurs
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/NativeCoreEnvironment.h et src/Emulation/src/NativeCoreEnvironment.cpp pour les dossiers, les annonces AV, les callbacks d’entrée, la latence et les diagnostics de osd-support.
  - [x] Modifier src/Emulation/CMakeLists.txt pour compiler le service commun.
  - [x] Créer puis supprimer build/emulation-native-environment-check après contrôle du chemin des callbacks, des dossiers, du format AV et des compteurs audio.
  - [x] Régénérer Emulation.dll dans le paquet Windows Release.

- [x] Configurations des variables natives de la première interface
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/NativeCoreOptions.h et src/Emulation/src/NativeCoreOptions.cpp pour les définitions, valeurs, chaînes empruntées et notifications de EmulationLegacyOptionHost.cs.
  - [x] Modifier NativeCoreEnvironment.h et NativeCoreEnvironment.cpp pour raccorder SetVariables, GetVariable et GetVariableUpdate.
  - [x] Modifier src/Emulation/CMakeLists.txt pour compiler le service commun.
  - [x] Créer puis supprimer build/emulation-native-options-check après contrôle des définitions, des valeurs configurées et du signal de mise à jour.
  - [x] Régénérer Emulation.dll dans le paquet Windows Release.

- [x] Options natives version 1 et 2
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreOptions.h et src/Emulation/src/NativeCoreOptions.cpp pour copier les définitions et catégories natives, fusionner les libellés localisés et relayer la visibilité et les valeurs.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreEnvironment.h et src/Emulation/src/NativeCoreEnvironment.cpp pour raccorder les commandes d’options et publier le catalogue.
  - [x] Créer puis supprimer build/emulation-native-modern-options-check après vérification des appels natifs et régénérer build/Release/Retro Media Lab/lib/Emulation.dll.

- [x] Interface de fichiers native version 3
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/NativeCoreFileSystem.h et src/Emulation/src/NativeCoreFileSystem.cpp pour les fichiers, dossiers, alias en lecture seule et les callbacks de osd-support.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreEnvironment.h, src/Emulation/src/NativeCoreEnvironment.cpp et src/Emulation/CMakeLists.txt pour publier la table native à la demande.
  - [x] Créer puis supprimer build/emulation-native-vfs-check après contrôle des appels et de l’isolation de deux sessions puis régénérer Emulation.dll dans le paquet Windows Release.

- [x] Changement de disque des cœurs natifs
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/NativeCoreDiskControl.h et src/Emulation/src/NativeCoreDiskControl.cpp pour les tables, la sélection, l’insertion et le rétablissement de l’état en cas d’échec selon osd-support.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreEnvironment.h, src/Emulation/src/NativeCoreEnvironment.cpp et src/Emulation/CMakeLists.txt pour raccorder les commandes de changement de disque.
  - [x] Créer puis supprimer build/emulation-native-disk-check après vérification des callbacks puis régénérer Emulation.dll dans le paquet Windows Release.

- [x] Cycle de vie des cœurs natifs
  - [x] Modifier NativeCoreLibrary.h et NativeCoreLibrary.cpp dans src/Emulation pour valider la version et l’identité avec la bibliothèque déjà ouverte.
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/NativeCoreSession.h et src/Emulation/src/NativeCoreSession.cpp pour installer les callbacks, charger le média, exécuter les trames et nettoyer dans l’ordre de osd-support. Modifier NativeCoreEnvironment.h et .cpp pour remonter les échecs des callbacks au service.
  - [x] Modifier src/Emulation/CMakeLists.txt pour compiler le service commun.
  - [x] Créer puis supprimer build/emulation-native-session-check après vérification sur un cœur temporaire et régénérer Emulation.dll dans le paquet Windows Release.

- [x] Raccordement des callbacks des sessions natives
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreSession.h et src/Emulation/src/NativeCoreSession.cpp pour associer les six callbacks à leur environnement, aux trames et à la lecture des entrées de la session.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreEnvironment.h pour associer les récepteurs audio et vidéo existants à la session propriétaire.
  - [x] Créer puis supprimer build/emulation-native-binding-check après vérification de deux cœurs chargés et de la libération des callbacks puis régénérer Emulation.dll en Release Windows.

- [x] Persistance de la mémoire des cœurs natifs
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreSession.h et src/Emulation/src/NativeCoreSession.cpp pour lire et restaurer une région de mémoire native avec contrôle de taille et écriture atomique.
  - [x] Créer puis supprimer build/emulation-native-save-memory-check après contrôle des sauvegardes et des fichiers invalides puis régénérer Emulation.dll en Release Windows.

- [x] Chargement des médias d’un sous-système natif
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreLibrary.h et src/Emulation/src/NativeCoreLibrary.cpp pour résoudre l’export de chargement spécial.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreSession.h et src/Emulation/src/NativeCoreSession.cpp pour appeler cet export avec des descripteurs contigus et conserver les données et emplacements vides jusqu’au déchargement.
  - [x] Créer puis supprimer build/emulation-native-subsystem-check après vérification des descripteurs et des chemins d’échec puis régénérer Emulation.dll en Release Windows.

- [x] Isolation des bibliothèques de cœurs natifs
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreLibrary.h et src/Emulation/src/NativeCoreLibrary.cpp pour copier le binaire dans un dossier détenu par la session et le retirer après déchargement.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreSession.h et src/Emulation/src/NativeCoreSession.cpp pour exposer ce choix aux adaptateurs qui utilisent une copie distincte sur osd-support.
  - [x] Créer puis supprimer build/emulation-native-isolation-check après vérification de deux sessions issues du même binaire et du nettoyage sur échec, puis régénérer Emulation.dll en Release Windows.

- [x] Descriptions natives des commandes, ports et plages mémoire
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreEnvironment.h et src/Emulation/src/NativeCoreEnvironment.cpp pour copier les descriptions UTF-8 et les métadonnées des tables ABI dans l’ordre de osd-support.
  - [x] Modifier src/Emulation/src/NativeCoreSession.cpp pour retirer les références empruntées au cœur après sa désinitialisation.
  - [x] Créer puis supprimer build/emulation-native-descriptors-check après vérification des commandes et des bornes puis régénérer Emulation.dll en Release Windows.

- [x] Annonces et capacités de l’environnement natif
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreEnvironment.h et src/Emulation/src/NativeCoreEnvironment.cpp pour conserver les annonces de géométrie, rotation et performances, et exposer les capacités d’entrée et interfaces optionnelles.
  - [x] Modifier NativeCoreEnvironment.h et .cpp pour copier les messages simples et étendus avec leur durée et leurs champs natifs.
  - [x] Créer puis supprimer build/emulation-native-queries-check après vérification de l’ABI puis régénérer Emulation.dll en Release Windows.

- [x] Commandes typées du client des cœurs hébergés
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostedCoreSession.h et src/Emulation/src/HostedCoreSession.cpp pour employer les formats binaires et JSON de osd-support pour les états, médias, contrôleurs, options et disques.
  - [x] Créer puis supprimer build/emulation-hosted-commands-check après contrôle des formats et des réponses invalides puis régénérer Emulation.dll en Release Windows.

- [x] PNG des thèmes d’origine
  - [x] Ajouter les PNG de src/RetroMediaLab.App/Theme de osd-support à src/App/Assets/Themes, dans les dossiers des thèmes correspondants, avec RetroMediaLab pour Base.
  - [x] Régénérer les dossiers Assets/Themes des paquets Debug et Release et vérifier le contenu des PNG d’origine.

Les 329 PNG de RetroMediaLab.App sur osd-support sont présents par contenu. Les 100 PNG des thèmes ajoutés sont rangés sous Assets/Themes, avec les aperçus Base dans RetroMediaLab. Les 595 PNG applicatifs sont identiques aux sources dans les paquets Debug et Release, dont 39 dans Assets/Icons.

- [x] Initialisation du client des cœurs avec sélection de disque
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostedCoreSession.h et src/Emulation/src/HostedCoreSession.cpp pour sérialiser la requête sans identifiant de moteur et décoder les champs de disque dans l’ordre de osd-support.
  - [x] Créer puis supprimer build/emulation-hosted-profile-check après vérification du protocole, puis régénérer Emulation.dll en Release Windows.

- [x] Suivi du disque actif dans le client hébergé
  - [x] Modifier src/Emulation/src/HostedCoreSession.cpp pour mettre à jour le disque actif après les commandes confirmées et lire l’index d’insertion dans slot.index selon osd-support.
  - [x] Créer puis supprimer build/emulation-hosted-disks-check après vérification des commandes et de leur effet, puis régénérer Emulation.dll en Release Windows.

- [x] Réinitialisation et arrêt des cœurs hébergés
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostedCoreSession.h et src/Emulation/src/HostedCoreSession.cpp pour exposer les commandes sans charge, le redémarrage du processus et la fermeture après arrêt prévue par osd-support.
  - [x] Modifier les mêmes fichiers pour prendre en charge les trois sources d’index utilisées à l’insertion dans osd-support.
  - [x] Créer puis supprimer build/emulation-hosted-lifecycle-check après vérification des échanges entre processus, puis régénérer Emulation.dll en Release Windows.

- [x] Réponses des trames des cœurs hébergés
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostedCoreSession.h et src/Emulation/src/HostedCoreSession.cpp pour que les profils Acorn et Commodore puissent reprendre le champ de sauvegarde et l’index d’insertion de osd-support.
  - [x] Créer puis supprimer build/emulation-hosted-frames-check pour vérifier les trois dispositions de réponse, la mémoire vidéo et la conservation des données déjà reçues, puis régénérer Emulation.dll en Release Windows.

- [x] Indications du panneau de réglage plein écran
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour afficher les touches de réglage, la navigation entre valeurs et le bouton de retour du C#, avec la molette sur le panneau.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour transmettre séparément les touches verticales et horizontales ainsi que la traduction Modifier.
  - [x] Créer puis supprimer build/emulation-osd-editor-check après vérification du panneau, puis régénérer les bibliothèques Release.

- [x] Enregistrement sur cassette depuis la session
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour transférer la préparation de la cassette et conserver le montage existant quand le chemin et le profil PCM correspondent.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour appeler le service de session après les dialogues.
  - [x] Créer puis supprimer build/emulation-tape-session-check après vérification du remploi de la cassette, puis régénérer le paquet Windows Release.

- [x] Sortie audio et état du tampon des machines
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h et HostProtocol.h pour partager le contrat optionnel de retour du tampon avec les cœurs natifs et hébergés.
  - [x] Créer src/Emulation/src/AudioOutput.h et AudioOutput.cpp et modifier src/Emulation/src/Host.cpp, Host.h et CMakeLists.txt pour gérer la sortie et publier son état depuis la session.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et src/App/CMakeLists.txt pour déléguer le flux et retirer Qt Multimedia des dépendances directes du thème.
  - [x] Créer puis supprimer build/emulation-audio-feedback-check pour vérifier la transmission au module via la session, puis régénérer le paquet Windows Release.

- [x] Retour du tampon sur la sortie audio Windows
  - [x] Créer puis supprimer build/emulation-active-audio-check pour envoyer du PCM silencieux via le SDK, relever la capacité et le remplissage transmis, et vérifier leur retrait après l’arrêt.

- [x] Latence audio minimale demandée par le cœur
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h et src/Emulation/src/AudioOutput.h et AudioOutput.cpp pour transmettre la demande et dimensionner le tampon Qt.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-active-audio-check pour vérifier la demande sur une sortie réelle.

- [x] Sélection de la sortie audio de la machine
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h et src/Emulation/src/Host.cpp pour compléter les choix déclarés et appliquer le périphérique choisi depuis la configuration du module.
  - [x] Modifier src/Emulation/src/AudioOutput.h et AudioOutput.cpp pour identifier et ouvrir la sortie choisie ou la sortie par défaut.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-audio-device-check après vérification des choix et de la bascule de sortie.

- [x] Volume et latence configurés par la machine
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h, src/Emulation/src/Host.cpp, AudioOutput.h et AudioOutput.cpp pour appliquer le volume et la plus grande des latences demandées par la machine et le cœur.
  - [x] Régénérer le paquet Windows Release, puis créer et supprimer build/emulation-audio-settings-check après vérification des mises à jour à chaud.

- [x] Ajustements d’image propres à chaque écran
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour identifier les treize écrans et lire puis écrire leurs ajustements dans processing.screenAdjustments.
  - [x] Modifier src/VideoPresentation/src/SoftwareProcessing.cpp et Presentation.cpp pour appliquer les ajustements de l’écran actif et les restrictions du C#.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour éditer le profil de l’écran actif dans le dialogue et l’OSD.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-screen-adjustments-check après vérification de la persistance et du rendu.

- [x] Choix des écrans et commandes de restauration
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour la sélection directe des treize écrans et la disponibilité des réglages.
  - [x] Modifier src/VideoPresentation/src/SoftwareProcessing.cpp pour appliquer la mise à l’échelle et la restauration selon l’écran actif.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App emulation-video-processing.json pour reprendre les libellés, la sélection et les réglages visibles du C#.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-screen-controls-check après vérification de la sélection, de la restauration et du rendu.

- [x] Entrées vidéo propres aux écrans simulés
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour enregistrer les entrées LCD, LED, OLED, plasma et projection aux chemins du C#.
  - [x] Modifier src/VideoPresentation/src/CrtConnection.h, CrtConnection.cpp, CrtProcessing.h, CrtProcessing.cpp et SoftwareProcessing.cpp pour partager le décodage du signal et appliquer la règle tramage composite et RF.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour afficher les réglages de l’entrée active dans le dialogue et l’OSD.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-panel-signal-check après vérification du rendu et des profils.

- [x] Définition de la dalle vidéo
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour conserver les dix définitions du C# dans leurs profils de signal.
  - [x] Modifier src/VideoPresentation/src/SoftwareProcessing.cpp pour transposer le double échantillonnage au plus proche de la dalle.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App emulation-video-fixed-pixel.json pour reprendre les choix d’origine dans le dialogue et l’OSD.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-panel-definition-check après vérification des profils et des pixels échantillonnés.

- [x] Rétroéclairage LCD à LED
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour lire et enregistrer le type de dalle, l’intensité et les quatre modes de rétroéclairage dans le profil LED.
  - [x] Modifier src/VideoPresentation/src/FixedPixelProcessing.h, FixedPixelProcessing.cpp et SoftwareProcessing.cpp pour appliquer l’éclairage de bord, direct ou par zones du C#.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App emulation-video-fixed-pixel.json pour exposer les réglages LED dans le dialogue et l’OSD.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-led-backlight-check après vérification des modes, des zones et du profil.

- [x] Séparation des couleurs d’un projecteur DLP mono-puce
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour mémoriser les cinq technologies du projecteur.
  - [x] Modifier src/VideoPresentation/src/PaperProjectionProcessing.h, PaperProjectionProcessing.cpp, SoftwareProcessing.h et SoftwareProcessing.cpp pour traiter les fronts en mouvement et invalider l’historique comme dans le C#.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App emulation-video-paper-projection.json pour exposer les choix dans le dialogue et l’OSD.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-dlp-check après vérification des trames et de la persistance.

- [x] Surfaces et optique de projection
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour enregistrer la surface et la lumière ambiante du C#.
  - [x] Modifier src/VideoPresentation/src/PaperProjectionProcessing.h, PaperProjectionProcessing.cpp et SoftwareProcessing.cpp pour reprendre les optiques par technologie et les surfaces du filtre C#.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App emulation-video-paper-projection.json pour exposer les surfaces et supprimer les réglages de projection absents du C#.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-projection-optics-check après vérification du rendu et du profil.

- [x] Dalles et sous-pixels OLED
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour les trois dalles et les sept structures sous processing.oled.
  - [x] Modifier src/VideoPresentation/src/FixedPixelProcessing.h, FixedPixelProcessing.cpp et SoftwareProcessing.cpp pour porter le limiteur, le rendu des sous-pixels et la luminance WOLED.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App emulation-video-fixed-pixel.json pour exposer les choix et retirer les commandes OLED génériques remplacées.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-oled-check après vérification des dalles et des structures.

- [x] Dalles LCD et LCD rétroéclairées
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour mémoriser les réglages LCD et LED et reprendre les anciennes valeurs compatibles.
  - [x] Modifier src/VideoPresentation/src/FixedPixelProcessing.h, FixedPixelProcessing.cpp et SoftwareProcessing.cpp pour reprendre la structure, la lumière et le temps de réponse des dalles du C#.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App emulation-video-fixed-pixel.json pour exposer les réglages du C#.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-lcd-panel-check après vérification du profil et du rendu.

- [x] Entrées vidéo OLED et projection
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour partager les entrées autorisées et normaliser les profils importés et enregistrés.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour utiliser les entrées autorisées dans le dialogue et l’OSD.
  - [x] Régénérer le paquet Windows Release et supprimer build/emulation-display-connections-check après la vérification temporaire du contrat.

- [x] Réglages et rendu du plasma
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour porter les structures de cellules et les définitions et entrées propres au plasma.
  - [x] Modifier src/VideoPresentation/src/PlasmaProcessing.h, PlasmaProcessing.cpp, FixedPixelProcessing.h, FixedPixelProcessing.cpp et SoftwareProcessing.cpp pour reprendre la chaîne plasma du C#.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App/emulation-video-plasma.json pour porter les choix du dialogue et de l’OSD.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-plasma-profile-check après contrôle du rendu et des profils.

- [x] Réglages de l’écran vectoriel
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reprendre les deux groupes et les curseurs du thème Base ainsi que l’ordre des réglages dans l’OSD.
  - [x] Modifier src/Translations/Catalogs/Constants/App/emulation-video-vector.json et les catalogues de langues correspondants pour reprendre les titres de groupes des RESX du C#.
  - [x] Modifier src/VideoPresentation/src/VectorProcessing.cpp pour normaliser le choix du phosphore avant le rendu et l’enregistrement.
  - [x] Régénérer le paquet Windows Release et supprimer build/emulation-vector-settings-check après le contrôle des profils.

- [x] Réglages VFD et afficheurs matriciels
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reprendre les groupes et curseurs VFD, matrice LED et matrice de points, et reproduire l’ordre et l’activation des réglages dans l’OSD.
  - [x] Modifier src/Translations/Catalogs/Constants/App/emulation-video-vfd.json, emulation-video-led-matrix.json et les catalogues de langues correspondants pour reprendre les titres C#.
  - [x] Régénérer le paquet Windows Release.

- [x] Ordre et visibilité des commandes vidéo de l’OSD
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reproduire la liste aplatie du C#, ses sections, l’ordre de ses commandes et leurs conditions.
  - [x] Modifier src/Translations/Catalogs/Constants/App/emulation-video-fixed-pixel.json pour ajouter le titre invariant LED.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-video-rows-check après vérification des commandes vidéo.

- [x] Répartition des réglages vidéo des modules dans l’OSD
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h, Host.h et src/Emulation/src/Host.cpp pour exposer une section par champ via le SDK, avec Affichage comme valeur implicite.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour aplatir les champs éditables des modules dans les sections Affichage, Image et Effets en respectant leur visibilité.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-module-video-rows-check après vérification de la répartition.

- [x] Aperçu et enregistrement des réglages de l’OSD
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour gérer la valeur provisoire et enregistrer la dernière modification avant de quitter la commande.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour appliquer l’aperçu uniquement aux réglages vidéo numériques, enregistrer le profil et restaurer la valeur enregistrée en cas d’échec.
  - [x] Régénérer le paquet Windows Release puis créer et supprimer build/emulation-osd-preview-check après vérification de l’aperçu, de l’enregistrement et du repli.

- [x] Remettre à jour les paquets du sélecteur Émulation
  - [x] Modifier src/App/CMakeLists.txt pour compiler EmulationPage.cpp avec le format objet étendu de MSVC.
  - [x] Régénérer build/Debug/Retro Media Lab/lib/Themes/RetroMediaLab.dll et build/Release/Retro Media Lab/lib/Themes/RetroMediaLab.dll avec le sélecteur à vignettes et vérifier les PNG installés dans Assets.

- [x] Aligner les commandes et l’ordre du menu plein écran sur osd-support
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour retirer les entrées vidéo supplémentaires, afficher l’état muet, reprendre les libellés des interrupteurs et présenter les sections intégrées dans l’ordre C#.

- [x] Libellé de la sortie audio dans le menu plein écran
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour exposer l’identifiant du champ de périphérique audio déclaré par le module.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour appliquer le libellé commun au groupe contenant ce champ.
  - [x] Modifier les catalogues App/emulation-osd.json pour reprendre le libellé de EmulationAudio.resx dans chaque langue fournie.
  - [x] Créer puis supprimer build/emulation-osd-audio-check pour vérifier le sous-menu Audio et le rafraîchissement du statut muet.

- [x] Disposition du menu plein écran selon osd-support
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour afficher un seul sous-menu, placer les menus courts sur leur ligne et recentrer l’éditeur en bas.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour déclarer les sous-menus qui occupent toute la hauteur dans le C#.
  - [x] Créer puis supprimer build/emulation-osd-panels-check pour vérifier les dimensions, la navigation et le retour depuis l’éditeur.

- [x] Réévaluation des menus après un changement de réglage
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour replier les niveaux masqués, repositionner la sélection et fermer l’éditeur dont le réglage disparaît.
  - [x] Créer puis supprimer build/emulation-osd-visibility-check pour vérifier ces transitions et l’enregistrement des changements.

- [x] Indicateur de chargement du shader en plein écran
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour partager l’indicateur animé et l’intégrer au calque OSD.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour transmettre le chargement au calque ouvert et utiliser l’indicateur autonome en son absence.
  - [x] Créer puis supprimer build/emulation-osd-shader-check pour vérifier le centrage et les transitions de visibilité.

- [x] Cache des shaders Direct3D 11 compilés à l’exécution
  - [x] Modifier src/VideoPresentation/src/Direct3DSurface.cpp pour lire et écrire le cache utilisateur vérifié SHA-256 de osd-support autour de la compilation HLSL.
  - [x] Créer puis supprimer build/emulation-d3d-shader-cache-check pour vérifier une compilation réelle, la réutilisation et la réparation automatique d’une entrée corrompue.

- [x] Modificateurs AltGr transmis à la machine
  - [x] Modifier src/App/Shared/Emulation/EmulationHostKeyboard.h et src/App/Shared/Emulation/EmulationHostKeyboard.cpp pour appliquer la fenêtre de détection de KeyboardModifierState.cs et réinitialiser son état au relâchement.
  - [x] Créer puis supprimer build/emulation-altgr-check pour distinguer un Ctrl généré par AltGr d’un Ctrl réellement maintenu.

- [x] AltGr dans la capture et l’exécution des raccourcis
  - [x] Modifier src/App/Shared/Emulation/EmulationHostKeyboard.h et .cpp pour partager la détection du Ctrl synthétique et la projection des modificateurs réels.
  - [x] Modifier src/App/Shared/Controls/EmulationShortcutCapture.h et src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour filtrer le Ctrl synthétique durant la capture et avant le déclenchement hôte.
  - [x] Créer puis supprimer build/emulation-altgr-shortcut-check pour vérifier la capture et les combinaisons avec un Ctrl réel.

- [x] Raccourcis MIDI du menu plein écran
  - [x] Créer src/Platform/include/RetroMediaLab/Platform/MidiInput.h, src/Platform/Windows/MidiInput.cpp et src/Platform/Unix/MidiInput.cpp et modifier src/Platform/CMakeLists.txt pour lire et relâcher les notes WinMM dans un service partagé.
  - [x] Modifier src/App/Shared/Emulation/EmulationPhysicalShortcuts.h, src/App/Shared/Controls/EmulationShortcutCapture.h et src/App/Themes/RetroMediaLab/Views/EmulationPreferencesDialog.cpp pour capturer les notes des commandes OSD avec le format du C#.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour activer la lecture MIDI en plein écran lorsqu’un raccourci OSD l’utilise.
  - [x] Créer puis supprimer build/emulation-midi-shortcut-check pour vérifier l’état des notes, les identifiants et la correspondance des raccourcis.

- [x] Répétition des directions dans l’éditeur plein écran
  - [x] Modifier src/App/Shared/Emulation/EmulationKeyboardShortcuts.h et EmulationPhysicalShortcuts.h pour identifier les quatre directions et appliquer les délais du C#.
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour publier l’état de l’éditeur.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour répéter uniquement les directions maintenues pendant l’édition et réinitialiser la répétition aux transitions.
  - [x] Créer puis supprimer build/emulation-osd-repeat-check pour vérifier la cadence, les transitions et l’arrêt au relâchement.

- [x] Vérifier le sélecteur Émulation dans le thème distribué
  - [x] Créer puis supprimer build/emulation-selector-check pour ouvrir l’onglet, contrôler les deux rangées, l’onglet Accueil et le repli du cadre dans le paquet Release.

- [x] Tampon audio partagé des modules
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/AudioBuffer.h et src/Emulation/src/AudioBuffer.cpp pour partager la rétention des trames, les compteurs, le vidage et la consommation native avec le SDK.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreFrames.h et src/Emulation/src/NativeCoreFrames.cpp pour utiliser la file commune dans les rappels audio et conserver les derniers échantillons.
  - [x] Modifier src/Emulation/CMakeLists.txt pour intégrer le tampon à la bibliothèque et à la cible du SDK.
  - [x] Créer puis supprimer build/emulation-audio-buffer-check pour vérifier la rétention, les compteurs et le raccordement des rappels.

- [x] Vidage des sorties du cœur natif
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreFrames.h et src/Emulation/src/NativeCoreFrames.cpp pour retirer la dernière trame vidéo en conservant sa séquence.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreEnvironment.h, src/Emulation/src/NativeCoreEnvironment.cpp et src/Emulation/src/NativeCoreSession.cpp pour vider les sorties au redémarrage, après restauration d’un état et lors de la fermeture du cœur.
  - [x] Créer puis supprimer build/emulation-native-output-check pour vérifier les transitions via les exports d’un cœur temporaire.

- [x] Historique de séparation des couleurs DLP
  - [x] Modifier src/VideoPresentation/src/PaperProjectionProcessing.h et .cpp pour invalider les trames en cache quand la configuration du projecteur change.
  - [x] Modifier src/VideoPresentation/src/SoftwareProcessing.cpp pour abandonner cet historique à la sortie du DLP, y compris lorsque le rendu saute les filtres.
  - [x] Créer puis supprimer build/emulation-dlp-history-check pour contrôler les transitions de configuration, de mode et de trame.

- [x] Pilotage partagé des vibrations de session
  - [x] Créer src/App/Shared/Emulation/EmulationControllerFeedback.h et .cpp pour agréger les sorties, arrêter les moteurs sur erreur et dédupliquer les diagnostics comme dans osd-support.
  - [x] Modifier src/App/CMakeLists.txt et src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour raccorder la vue au contrôleur partagé.
  - [x] Créer puis supprimer build/emulation-controller-routing-check pour vérifier les moteurs, la désactivation et l’isolation d’une erreur.

- [x] Image de la session après extinction ou remplacement du cœur
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour publier une révision qui avance seulement quand le nouveau cœur remplace l’ancien.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour effacer l’image et les statistiques lors des transitions de la session, et conserver la trame en pause.
  - [x] Modifier src/VideoPresentation/src/Presentation.cpp pour que la disparition d’une trame efface l’image et invalide le traitement différé une seule fois.
  - [x] Créer puis supprimer build/emulation-frame-lifecycle-check pour vérifier l’effacement et le retour d’une trame.

- [x] Valeurs des réglages des modules dans le menu plein écran
  - [x] Modifier src/App/Shared/Emulation/ModuleNumericSetting.h pour partager le libellé d’un choix et son unité entre les vues.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour afficher les unités et conserver les sélections ou curseurs sans éditeur en lecture seule comme dans osd-support.
  - [x] Créer puis supprimer build/emulation-osd-module-value-check pour vérifier les valeurs détachées de leur choix et les unités numériques.

- [x] Actualisation des commandes de la machine après enregistrement
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/InputSettings.h pour exposer la mise à jour des commandes à chaud selon le contrat IEmulationInput du C#.
  - [x] Modifier src/Emulation/src/Host.cpp pour transmettre la configuration enregistrée aux machines actives avant les options du cœur.
  - [x] Créer puis supprimer build/emulation-live-bindings-check pour vérifier l’enregistrement, l’ordre des appels et la distinction machine active ou arrêtée.

- [x] Sélection des configurations dans l’onglet Émulation
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour retenir l’identifiant de la configuration ouverte au clic, ouvrir la configuration choisie au double-clic et respecter la largeur minimale du bouton Ouvrir.

- [x] Validation des raccourcis Émulation
  - [x] Modifier src/App/Shared/Emulation/EmulationPhysicalShortcuts.h pour écarter les sources physiques invalides et reconnaître les modificateurs seuls du C#.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPreferencesDialog.cpp pour signaler les raccourcis illisibles avant l’enregistrement et limiter la saisie Qt à une combinaison simultanée.
  - [x] Modifier src/App/Shared/Controls/EmulationShortcutCapture.h pour terminer la capture au relâchement d’un raccourci composé uniquement de modificateurs.
  - [x] Créer puis supprimer build/emulation-shortcut-validation-check pour vérifier la capture et la correspondance des modificateurs, le rejet des sources physiques invalides et les raccourcis système.

- [x] Lancement et extensions de contenu des cœurs hébergés
  - [x] Modifier src/Emulation/src/HostedCoreTransport.cpp et src/Emulation/include/RetroMediaLab/Emulation/HostedCoreTransport.h pour utiliser le dossier de l’exécutable hôte par défaut et conserver le dossier explicite.
  - [x] Modifier src/Emulation/src/HostedCoreSession.cpp pour dédoublonner les extensions annoncées sans tenir compte de la casse, dans l’ordre reçu.
  - [x] Créer puis supprimer build/emulation-host-directory-check pour vérifier le dossier de lancement et la liste des extensions avec un hôte temporaire.

- [x] Reprise des cœurs natifs après une demande d’arrêt
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreEnvironment.h et src/Emulation/src/NativeCoreEnvironment.cpp pour réinitialiser la demande d’arrêt avec les sorties.
  - [x] Modifier src/Emulation/src/NativeCoreSession.cpp pour appliquer cette remise à zéro après réinitialisation, restauration réussie et fermeture du cœur.
  - [x] Créer puis supprimer build/emulation-native-shutdown-check pour vérifier la reprise sur un cœur temporaire et la conservation de l’état si la restauration échoue.

- [x] Densité d’écran de la vidéo émulée
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour recalculer l’image à un changement de densité et peindre les surimpressions matérielles à la résolution physique.
  - [x] Créer puis supprimer build/emulation-video-density-check pour vérifier le recalcul de la vidéo en pause sous Qt.

- [x] Historique vidéo après la modification d’un profil
  - [x] Modifier src/VideoPresentation/src/Presentation.cpp pour initialiser la rémanence avec l’image affichée après remise à zéro de l’historique.
  - [x] Créer puis supprimer build/emulation-video-history-check pour vérifier la transition d’image après un changement de profil LCD en pause.

- [x] Proportions de l’écran et du masque CRT
  - [x] Modifier src/VideoPresentation/src/SoftwareProcessing.h et SoftwareProcessing.cpp pour résoudre les proportions de la trame et dimensionner le masque CRT avec le ratio d’écran.
  - [x] Modifier src/VideoPresentation/src/Presentation.cpp pour ajuster l’image dans l’écran selon FitDisplay, y compris au format carré et pour les trames entrelacées.
  - [x] Modifier src/VideoPresentation/src/SoftwareFrameWorker.h pour conserver les proportions de la trame sur le chemin asynchrone.
  - [x] Créer puis supprimer build/emulation-video-aspect-check pour contrôler le cadrage 4:3 et le ratio entrelacé.

- [x] Choix des proportions par technologie vidéo
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h, src/VideoPresentation/src/Presentation.cpp et src/VideoPresentation/src/SoftwareProcessing.cpp pour partager les choix et les dimensions des rapports d’écran.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour actualiser le sélecteur lorsque la technologie change et conserver un rapport mémorisé lors des changements temporaires.

- [x] Trame commune au tube CRT et au masque phosphore
  - [x] Modifier src/VideoPresentation/src/CrtProcessing.h, src/VideoPresentation/src/CrtProcessing.cpp et src/VideoPresentation/src/SoftwareProcessing.cpp pour partager la courbure, le faisceau et les paramètres de signal entre les deux passes.
  - [x] Créer puis supprimer build/emulation-crt-mask-check pour vérifier les pixels hors du contour du tube et le rendu du masque à l’intérieur.

- [x] Rendu CRT Direct3D 11
  - [x] Créer src/VideoPresentation/shaders/crt.glsl avec les passes du tube et du masque de osd-support.
  - [x] Modifier src/VideoPresentation/src/CrtProcessing.h, CrtProcessing.cpp et HardwareSamplingShader.h pour partager la préparation des paramètres CRT avec Direct3D 11.
  - [x] Modifier src/VideoPresentation/src/Direct3DSurface.h, Direct3DSurface.cpp, Presentation.cpp et include/RetroMediaLab/VideoPresentation/Presentation.h pour raccorder le rendu, la capture et le repli logiciel.
  - [x] Créer puis supprimer build/emulation-crt-d3d-check pour vérifier le rendu et la capture avec Direct3D 11.
  - [x] Créer src/VideoPresentation/shaders/crt.frag et modifier src/VideoPresentation/CMakeLists.txt pour compiler séparément la passe CRT et conserver le programme de mise à l’échelle existant.

- [x] Rendu CRT Vulkan
  - [x] Modifier src/VideoPresentation/src/VulkanSurface.h et VulkanSurface.cpp pour créer la passe CRT dans le cache de pipelines, l’utiliser pour la trame et conserver le pipeline distinct des surimpressions.
  - [x] Modifier src/VideoPresentation/src/Presentation.cpp pour partager la préparation des trames CRT avec Direct3D et raccorder la capture ainsi que le repli logiciel.
  - [x] Créer puis supprimer build/emulation-crt-vulkan-check pour vérifier le rendu et la capture sous Vulkan.

- [x] Rendu CRT OpenGL
  - [x] Modifier src/VideoPresentation/shaders/crt.glsl, crt.frag et CMakeLists.txt pour partager le shader CRT avec OpenGL.
  - [x] Modifier src/VideoPresentation/src/OpenGlSurface.h, OpenGlSurface.cpp et Presentation.cpp pour utiliser ce shader à la présentation et à la capture avec le repli logiciel.
  - [x] Créer puis supprimer build/emulation-crt-opengl-check pour vérifier le rendu et la capture OpenGL.

- [x] Détramage avant rendu CRT matériel
  - [x] Modifier src/VideoPresentation/src/SoftwareProcessing.h et SoftwareProcessing.cpp pour partager la résolution des réglages de restauration actifs.
  - [x] Modifier src/VideoPresentation/src/Presentation.cpp pour appliquer le détramage autorisé avant le traitement de la connexion et conserver la passe matérielle lorsque le profil contient des réglages réservés au mode Normal.
  - [x] Créer puis supprimer build/emulation-crt-restoration-check pour vérifier le détramage numérique, son exclusion en composite et les réglages réservés au mode Normal.

- [x] Netteté du CRT matériel
  - [x] Modifier src/VideoPresentation/shaders/image-adjustments.glsl et crt.glsl pour appliquer le filtre de netteté commun avant le décodage du tube et le masque phosphore.
  - [x] Modifier src/VideoPresentation/src/OpenGlSurface.cpp et Presentation.cpp pour transmettre la netteté au shader CRT et conserver la passe matérielle avec ce réglage.
  - [x] Créer puis supprimer build/emulation-crt-sharpness-check pour vérifier le renforcement et l’adoucissement sur une mire.

- [x] État du processus des cœurs hébergés
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h pour exposer les états de processus communs du SDK.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostedCoreTransport.h et src/Emulation/src/HostedCoreTransport.cpp pour relever le PID au lancement et détecter la sortie du processus.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostedCoreSession.h et src/Emulation/src/HostedCoreSession.cpp pour exposer le PID conservé et distinguer la sortie de l’échec du transport.
  - [x] Créer puis supprimer build/emulation-host-process-state-check pour vérifier les transitions et la conservation du PID.

- [x] Informations d’exécution communes du cœur
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h pour exposer le contrat facultatif et les données d’exécution du cœur.
  - [x] Modifier HostedCoreSession.h et HostedCoreSession.cpp pour publier les mesures reçues du processus hôte.
  - [x] Modifier NativeCoreSession.h, NativeCoreSession.cpp, NativeCoreFrames.h et NativeCoreFrames.cpp pour publier les mesures du cœur exécuté dans le processus courant.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour lire l’horloge nominale de ce contrat avec le repli existant.
  - [x] Créer puis supprimer build/emulation-core-runtime-status-check pour vérifier les mesures et le cycle du cœur hébergé.

- [x] Retour de force des cœurs natifs
  - [x] Modifier src/Emulation/src/NativeCoreSession.cpp et src/Emulation/include/RetroMediaLab/Emulation/NativeCoreSession.h pour raccorder le rappel natif à un accumulateur propre à la session, publier son état et effacer celui du port réaffecté.
  - [x] Créer puis supprimer build/emulation-native-feedback-check pour vérifier la vibration et les mesures natives en chargeant une DLL temporaire.

- [x] Retour de force des cœurs hébergés
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostedCoreSession.h et src/Emulation/src/HostedCoreSession.cpp pour fournir les effets du processus actif aux adaptateurs et écarter ceux du port après réaffectation réussie.
  - [x] Créer puis supprimer build/emulation-hosted-feedback-check pour vérifier les effets reçus, la réaffectation, le refus distant et la sortie du processus.

- [x] Réinitialisation du retour de force
  - [x] Modifier src/Emulation/src/NativeCoreSession.cpp et src/Emulation/src/HostedCoreSession.cpp pour libérer les effets après réinitialisation et restauration réussies.
  - [x] Créer puis supprimer build/emulation-native-reset-feedback-check et build/emulation-hosted-reset-feedback-check pour vérifier les transitions.

- [x] Sorties du cœur hébergé après une commande de cycle de vie
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/HostedCoreSession.h et src/Emulation/src/HostedCoreSession.cpp pour invalider les sorties après réinitialisation, restauration ou arrêt confirmé.
  - [x] Créer puis supprimer build/emulation-hosted-output-lifecycle-check pour vérifier la dernière image, le retour de force et le statut après ces commandes.

- [x] Table et événements du clavier natif
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/InputProcessing.h et src/Emulation/src/InputProcessing.cpp pour exposer les correspondances natives de osd-support.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreSession.h, src/Emulation/src/NativeCoreSession.cpp et src/Emulation/src/NativeCoreEnvironment.cpp pour transmettre les événements et réinitialiser leur historique à la fermeture.
  - [x] Créer puis supprimer build/emulation-native-keyboard-session-check pour exercer le rappel avec la table commune.

- [x] Instantané des entrées pour les cœurs natifs
  - [x] Modifier src/Emulation/src/InputProcessing.cpp pour alimenter la table des événements et la lecture par interrogation à partir des mêmes correspondances.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreSession.h et src/Emulation/src/NativeCoreSession.cpp pour transmettre clavier, souris et contrôleurs depuis un instantané.
  - [x] Créer puis supprimer build/emulation-native-input-snapshot-check pour vérifier les rappels du cœur.

- [x] Options à chaud des cœurs natifs
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreSession.h et src/Emulation/src/NativeCoreSession.cpp pour publier les options actives et relayer les changements validés ainsi que les erreurs du cœur.
  - [x] Créer puis supprimer build/emulation-native-session-options-check pour vérifier les changements depuis le SDK jusqu’au cœur.

- [x] Commandes de disque dans la session native
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreSession.h et src/Emulation/src/NativeCoreSession.cpp pour exposer le statut, la sélection, l’insertion et l’éjection avec le contrôleur de disque existant.
  - [x] Créer puis supprimer build/emulation-native-session-disk-check pour vérifier les opérations à travers un cœur natif.

- [x] Installation sans dossier de modules vide
  - [x] Modifier CMakeLists.txt pour omettre la création systématique du dossier Modules tant qu’aucun module ne fait partie du paquet.
  - [x] Supprimer build/Release/Retro Media Lab/Modules et build/Debug/Retro Media Lab/Modules lorsqu’ils sont vides et vérifier le paquet Release après installation.

- [x] Définition de dalle du papier électronique
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour lire et enregistrer la définition sélectionnée.
  - [x] Modifier src/VideoPresentation/src/SoftwareProcessing.cpp pour appliquer la grille du papier électronique dans la passe de rééchantillonnage existante.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour exposer la définition dans le dialogue et l’OSD avec les libellés existants.
  - [x] Créer puis supprimer build/emulation-epaper-definition-check pour contrôler le profil et la grille de rendu.

- [x] Définitions de grille des afficheurs matriciels
  - [x] Modifier Presentation.h et Presentation.cpp dans src/VideoPresentation pour porter les neuf résolutions fixes et leur persistance.
  - [x] Modifier DotMatrixProcessing.cpp, LedMatrixProcessing.cpp et VfdProcessing.cpp dans src/VideoPresentation/src pour appliquer la grille choisie.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et src/Translations/Catalogs/Constants/App/emulation-video-fixed-pixel.json pour exposer les réglages et leurs dimensions.
  - [x] Créer puis supprimer build/emulation-matrix-definition-check pour contrôler les grilles et la persistance.

- [x] Échantillonnage des cellules LED depuis la trame source
  - [x] Modifier src/VideoPresentation/src/LedMatrixProcessing.cpp et src/VideoPresentation/src/SoftwareProcessing.cpp pour appliquer la prise centrale et les quatre prises de réduction du C# avant toute mise à l’échelle.
  - [x] Créer puis supprimer build/emulation-led-source-sampling-check pour vérifier la restitution de détails qui disparaîtraient dans le viewport.

- [x] Couleur rouge/vert et cellules LED à coins arrondis
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour exposer et enregistrer RedGreen et RoundedSquare.
  - [x] Modifier src/VideoPresentation/src/LedMatrixProcessing.cpp pour appliquer les deux canaux et la géométrie des coins du C#.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App/emulation-video-led-matrix.json pour proposer les réglages dans la fenêtre et dans l’OSD.
  - [x] Créer puis supprimer build/emulation-led-matrix-choices-check pour contrôler le rendu et la persistance.

- [x] Composition des matrices LED selon osd-support
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h et src/VideoPresentation/src/Presentation.cpp pour conserver couleur, définition et forme dans le profil.
  - [x] Modifier src/VideoPresentation/src/LedMatrixProcessing.cpp, LedMatrixProcessing.h et SoftwareProcessing.cpp pour reprendre la composition et sa transition selon la taille des cellules.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reprendre les trois choix du C# dans le dialogue et dans l’OSD.
  - [x] Créer puis supprimer build/emulation-led-composition-check pour vérifier les couleurs à faible agrandissement et le rendu des formes.

- [x] Rendu matériel des matrices LED
  - [x] Créer src/VideoPresentation/shaders/led-matrix.glsl et l’inclure dans la passe de rendu d’affichage et les ressources CMake.
  - [x] Créer src/VideoPresentation/src/DisplayShaderParameters.h et modifier OpenGlSurface, Direct3DSurface, VulkanSurface et HardwareSamplingShader dans le même dossier pour relayer les paramètres de grille.
  - [x] Modifier src/VideoPresentation/src/Presentation.cpp et include/RetroMediaLab/VideoPresentation/Presentation.h pour sélectionner la passe matérielle et conserver le rendu logiciel des combinaisons non prises en charge.
  - [x] Créer puis supprimer build/emulation-led-hardware-check pour comparer le rendu des moteurs disponibles au rendu logiciel.

- [x] Portage des cellules VFD de osd-support
  - [x] Modifier Presentation.h et Presentation.cpp dans src/VideoPresentation pour enregistrer les six couleurs et les trois formes.
  - [x] Modifier src/VideoPresentation/src/{VfdProcessing,LedMatrixProcessing,MatrixCells,SoftwareProcessing} pour partager les prises sur la trame source et composer les cellules VFD.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App/emulation-video-vfd.json pour reprendre les trois commandes du C#.
  - [x] Créer src/VideoPresentation/shaders/vfd-cells.glsl et raccorder la passe VFD aux paramètres de rendu Direct3D 11, Vulkan et OpenGL.
  - [x] Créer puis supprimer build/emulation-vfd-cells-check pour vérifier les profils et la composition.

- [x] Matrices de points réfléchissantes, rétroéclairées et émissives
  - [x] Modifier Presentation.h et Presentation.cpp dans src/VideoPresentation pour enregistrer la technologie et les réglages de grille du C#.
  - [x] Modifier src/VideoPresentation/src/{DotMatrixProcessing,MatrixCells,SoftwareProcessing} pour appliquer le filtre optique de osd-support.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App/emulation-video-dot-matrix.json pour reprendre les choix du C#.
  - [x] Créer src/VideoPresentation/shaders/dot-matrix.glsl et l’intégrer à la passe partagée des moteurs matériels.
  - [x] Créer puis supprimer build/emulation-dot-matrix-optics-check pour comparer les trois technologies sur les moteurs disponibles.

- [x] Technologies LCD, LED et VFD des afficheurs à segments
  - [x] Modifier Presentation.h, Presentation.cpp et SegmentDisplayProcessing.cpp dans src/VideoPresentation pour enregistrer la technologie et composer le rendu logiciel par optique.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App/emulation-video-segment-display.json pour reprendre les choix de technologie et de couleur.
  - [x] Créer puis supprimer build/emulation-segment-optics-check pour vérifier la persistance et les trois compositions.

- [x] Niveaux de segments natifs dans les modules
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h pour exposer les niveaux associés à une trame par une interface facultative.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h pour transmettre ensemble la trame et ses niveaux.
  - [x] Modifier src/VideoPresentation/src/{Presentation.cpp,SoftwareFrameWorker.h,SoftwareProcessing.h,SoftwareProcessing.cpp,SegmentDisplayProcessing.h,SegmentDisplayProcessing.cpp} pour composer les niveaux bruts et réinitialiser le suivi temporel.
  - [x] Créer puis supprimer build/emulation-native-segment-check pour vérifier les niveaux et le retour au traitement matriciel.

- [x] Rendu matériel des segments natifs
  - [x] Créer src/VideoPresentation/shaders/native-segments.glsl et le raccorder à Direct3D 11, Vulkan et OpenGL.
  - [x] Modifier src/VideoPresentation/src/{Presentation.cpp,SegmentDisplayProcessing.cpp,SegmentDisplayProcessing.h,DisplayShaderParameters.h,HardwareSamplingShader.h,OpenGlSurface.cpp} pour transmettre niveaux et teinte et conserver le repli logiciel.
  - [x] Créer puis supprimer build/emulation-native-segment-gpu-check pour comparer les niveaux et vérifier le halo VFD.

- [x] Rendu matériel des segments reconstruits depuis le raster
  - [x] Modifier `src/VideoPresentation/src/{SegmentDisplayProcessing,DisplayShaderParameters,HardwareSamplingShader,Presentation,OpenGlSurface}` pour partager la géométrie des segments et fournir la trame précédente au filtre matériel.
  - [x] Créer `src/VideoPresentation/shaders/raster-segments.glsl` et modifier le programme de rendu commun, `src/VideoPresentation/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` pour raccorder la reconstruction.
  - [x] Créer puis supprimer `build/emulation-raster-segment-check` pour vérifier le rendu et la réponse temporelle sur les moteurs disponibles.

- [x] Afficheurs à papier électronique de osd-support
  - [x] Modifier src/VideoPresentation/src/{PaperProjectionProcessing,SoftwareProcessing,Presentation} et Presentation.h pour porter les six encres, la cadence de rafraîchissement et la trace de pigment.
  - [x] Créer src/VideoPresentation/shaders/epaper.glsl et modifier les paramètres du shader partagé, OpenGlSurface.cpp et les manifestes de compilation pour intégrer le rendu matériel.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et les catalogues App/emulation-video-paper-projection.json pour afficher les modes et la définition du C# dans le dialogue et l’OSD.
  - [x] Créer puis supprimer build/emulation-epaper-check pour vérifier les pigments et la cadence en logiciel et sur les moteurs matériels.

- [x] Projection vidéo matérielle de osd-support
  - [x] Modifier src/VideoPresentation/src/{PaperProjectionProcessing,Presentation} pour partager les paramètres optiques entre moteurs et préparer le signal et la séparation colorée DLP.
  - [x] Créer src/VideoPresentation/shaders/projection.glsl et modifier le programme commun, les paramètres de rendu et les manifestes Qt et Visual Studio.
  - [x] Créer puis supprimer build/emulation-projection-gpu-check pour vérifier l’optique et la séparation colorée sur les moteurs disponibles.

- [x] Rendu plasma matériel de osd-support
  - [x] Modifier src/VideoPresentation/src/{PlasmaProcessing,Presentation} pour partager le réglage du phosphore, la mesure de luminance et l’historique des trames.
  - [x] Créer src/VideoPresentation/shaders/{panel-grid,plasma}.glsl et modifier le shader commun, les paramètres de rendu et les manifestes Qt et Visual Studio.
  - [x] Créer puis supprimer build/emulation-plasma-gpu-check pour vérifier les structures, la limitation de luminosité et la persistance.

- [x] Afficheur vectoriel natif et logiciel de osd-support
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h et src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour joindre les segments natifs à la trame sans changer le contrat binaire existant.
  - [x] Modifier src/VideoPresentation/src/{VectorProcessing,SoftwareProcessing,SoftwareFrameWorker,Presentation} et Presentation.h pour composer le faisceau, la courbure et la rémanence.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et le profil JSON pour exposer les trois réglages du C#.
  - [x] Créer puis supprimer build/emulation-vector-check pour vérifier les segments natifs et le chemin raster logiciel.

- [x] Afficheur vectoriel raster matériel de osd-support
  - [x] Modifier src/VideoPresentation/src/{VectorProcessing,Presentation,DisplayShaderParameters,HardwareSamplingShader,OpenGlSurface} pour partager le faisceau et fournir la trame raster précédente au shader.
  - [x] Créer src/VideoPresentation/shaders/vector-raster.glsl et modifier le shader commun et les manifestes Qt et Visual Studio.
  - [x] Utiliser la sonde build/emulation-vector-check du bloc précédent pour vérifier aussi l’afficheur raster matériel.

- [x] Rendu matériel des dalles LCD et OLED de osd-support
  - [x] Modifier src/VideoPresentation/src/{FixedPixelProcessing,Presentation} pour partager les paramètres de dalle, préparer le rétroéclairage local et transmettre l’historique LCD.
  - [x] Créer src/VideoPresentation/shaders/fixed-pixel.glsl et modifier le shader et ses paramètres partagés, les moteurs matériels et les manifestes Qt et Visual Studio.
  - [x] Créer puis supprimer build/emulation-fixed-pixel-check pour vérifier l’éclairage local, les sous-pixels, le limiteur OLED et la réponse LCD.
  - [x] Modifier src/VideoPresentation/src/Direct3DSurface.cpp pour réduire le temps de compilation de secours du shader d’affichage et vérifier l’initialisation à froid.

- [x] Décodage matériel du signal de dalle de osd-support
  - [x] Modifier src/VideoPresentation/src/Presentation.cpp pour transmettre le signal synchronisé à la dalle LCD et à la projection selon les conditions C#.
  - [x] Modifier src/VideoPresentation/shaders/{panel-grid,fixed-pixel,projection}.glsl pour décoder la source et son historique avant les réglages d’image.
  - [x] Créer puis supprimer build/emulation-panel-signal-check pour vérifier le rendu des décodeurs et de la désynchronisation.

- [x] Précompilation du shader d’affichage Direct3D 11
  - [x] Modifier src/VideoPresentation/CMakeLists.txt pour produire et intégrer le DXBC avec les options compatibles avec le shader complet.
  - [x] Modifier src/VideoPresentation/src/Direct3DSurface.cpp pour charger ce bytecode avec les liaisons du paquet Qt.
  - [x] Modifier src/VideoPresentation/shaders/crt.frag pour échantillonner explicitement le niveau de texture utilisé et éviter le déroulage imposé par les gradients implicites.
  - [x] Créer puis supprimer build/emulation-precompiled-shader-check pour vérifier le paquet Release et l’initialisation du rendu à froid.

- [x] Nettoyage des dossiers de session du SDK
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/SessionDirectories.h et src/Emulation/src/SessionDirectories.cpp pour retirer les dossiers de session possédés, y compris les dossiers générés à nom GUID avec fichiers en lecture seule.
  - [x] Modifier src/Emulation/CMakeLists.txt et régénérer RetroMediaLab.vcxproj et RetroMediaLab.vcxproj.filters pour exposer le service.
  - [x] Créer puis supprimer build/emulation-session-directories-check pour vérifier les dossiers générés, la portée du nettoyage et les liens.

- [x] Construction des blocs matériels et des choix audio numériques de osd-support
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/HardwareSettings.h et src/Emulation/src/HardwareSettings.cpp pour fournir les blocs CPU, mémoire et résolution, les champs d’information, les choix et les interrupteurs.
  - [x] Créer les catalogues App/emulation-hardware.json dans src/Translations/Catalogs/Constants et chaque dossier de langue avec les libellés C# et l’invariant RAM.
  - [x] Modifier src/Emulation/CMakeLists.txt et régénérer RetroMediaLab.vcxproj et RetroMediaLab.vcxproj.filters pour exposer les nouveaux constructeurs.
  - [x] Créer puis supprimer build/emulation-hardware-settings-check pour vérifier la description, les choix, les identifiants persistés et les unités.

- [x] Éditeur numérique des réglages de module
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/{Sdk,HardwareSettings}.h et src/Emulation/src/HardwareSettings.cpp pour exposer le champ numérique et conserver les anciens identifiants du SDK.
  - [x] Créer src/App/Shared/Controls/ModuleNumberEdit.h et modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reproduire la saisie contrainte et les pas clavier du C#.
  - [x] Régénérer RetroMediaLab.vcxproj et RetroMediaLab.vcxproj.filters, puis créer et supprimer build/emulation-number-edit-check pour vérifier la saisie et la valeur transmise.

- [x] Aides détaillées et cartes compactes des réglages de module
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/{Sdk,Host,HardwareSettings}.h et src/Emulation/src/{Host,HardwareSettings}.cpp pour exposer les métadonnées facultatives sans changer les descripteurs existants.
  - [x] Créer src/App/Themes/RetroMediaLab/Views/ModuleSettingPresentation.h et modifier EmulationPage.cpp pour afficher les aides et regrouper les cartes compactes selon la largeur.
  - [x] Régénérer RetroMediaLab.vcxproj et RetroMediaLab.vcxproj.filters et créer puis supprimer build/emulation-module-presentation-check pour vérifier les cartes et l’aide dans Qt.

- [x] Fichiers de configuration des machines
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/ConfigurationFiles.h et src/Emulation/src/ConfigurationFiles.cpp pour porter les chemins, les documents JSON, l’accès coordonné sous Windows et l’écriture atomique.
  - [x] Modifier src/Emulation/CMakeLists.txt et régénérer RetroMediaLab.vcxproj et RetroMediaLab.vcxproj.filters.
  - [x] Créer puis supprimer build/emulation-configuration-files-check pour vérifier les chemins, l’ordre de lecture, le verrou et l’annulation d’une écriture.

- [x] Messages structurés du socle d’émulation
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/Messages.h et src/Emulation/src/Messages.cpp pour porter les codes, contextes, causes et règles d’enveloppement d’erreurs du C#.
  - [x] Modifier src/Emulation/src/Host.cpp et src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour transmettre puis afficher les erreurs structurées à l’ouverture et au lancement.
  - [x] Créer les catalogues App/emulation-errors.json dans src/Translations/Catalogs/Constants et chaque dossier de langue avec les clés et textes C#.
  - [x] Modifier src/Emulation/CMakeLists.txt et régénérer RetroMediaLab.vcxproj et RetroMediaLab.vcxproj.filters.
  - [x] Créer puis supprimer build/emulation-messages-check pour vérifier les codes, le contexte, la cause, le texte natif et l’annulation.

- [x] Dialogue détaillé des erreurs d’émulation
  - [x] Créer src/App/Themes/RetroMediaLab/Views/EmulationErrorDialog.h et EmulationErrorDialog.cpp pour afficher le titre, le message, les détails et les icônes des médias du dialogue C#.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour respecter les cibles des messages pendant l’ouverture et les commandes de session.
  - [x] Modifier src/App/CMakeLists.txt et régénérer RetroMediaLab.vcxproj et RetroMediaLab.vcxproj.filters pour inclure le dialogue.
  - [x] Créer puis supprimer build/emulation-error-dialog-check pour vérifier les cibles, les détails et les icônes dans Qt.

- [x] Diagnostics des médias et des états sauvegardés
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour faire remonter les erreurs des commandes de démarrage, de médias et d’états sauvegardés en préservant les causes et les annulations.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/StateFile.h et src/Emulation/src/StateFile.cpp pour exposer une lecture vérifiée distinguant un état invalide, incompatible ou inaccessible.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour utiliser les commandes de session et les messages localisés correspondants.
  - [x] Créer puis supprimer build/emulation-state-errors-check pour vérifier la lecture et les codes d’erreur.

- [x] Validation des erreurs à travers un module chargé
  - [x] Créer puis supprimer build/emulation-module-errors-check avec une DLL de test et son manifeste, puis vérifier les commandes du SDK et le même dialogue Qt avec les exceptions émises par la DLL.

- [x] Présentation du transport de cassette de RetroMediaLab
  - [x] Créer src/App/Shared/Emulation/TapeTransportPresentation.h pour calculer les états des six commandes selon les règles C#.
  - [x] Créer src/App/Themes/RetroMediaLab/Views/TapeTransportButton.h pour dessiner les glyphes, les bordures et le clignotement du thème.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour utiliser les commandes de transport et les états du SDK, puis régénérer RetroMediaLab.vcxproj et RetroMediaLab.vcxproj.filters.
  - [x] Créer puis supprimer build/emulation-tape-buttons-check pour vérifier les états et le rendu Qt.

- [x] Préparation des médias lors du montage interactif
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h pour déclarer la préparation facultative d’un fichier par l’adaptateur du module.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour préparer le média avant son insertion et transmettre le résultat au parcours de montage commun.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour utiliser ce parcours à l’ouverture d’un fichier de média.
  - [x] Créer puis supprimer build/emulation-media-prepare-check pour vérifier le chargement d’un module et l’insertion avec préparation, refus et annulation.

- [x] Dix emplacements de sauvegarde par configuration de machine
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour localiser, dater, écrire et relire les dix emplacements en préservant la sauvegarde rapide existante.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour présenter les dix emplacements à l’écriture et à la lecture dans la barre et dans l’OSD, et conserver les raccourcis rapides.
  - [x] Créer puis supprimer build/emulation-state-slots-check avec une DLL de test pour vérifier l’isolation des emplacements et des configurations, le rechargement et la préservation après échec.

- [x] Fermeture d’une session après une erreur du moteur
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour retirer la session fermée, arrêter sa sortie audio et tenter l’arrêt du moteur une seule fois.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour retirer l’onglet et signaler un arrêt en erreur.
  - [x] Créer puis supprimer build/emulation-session-close-check avec une DLL de test qui échoue à l’arrêt, pour vérifier la fermeture et la réouverture.

- [x] Vérification des emplacements dans le thème
  - [x] Créer puis supprimer build/emulation-state-menus-check pour charger le thème Release et vérifier les dix choix de sauvegarde et les dix choix de chargement avec une machine de test.

- [x] Libération du moteur après un échec au démarrage
  - [x] Modifier src/Emulation/src/Host.cpp pour arrêter et détruire le moteur dont le démarrage échoue, garder les périphériques configurables et recréer le moteur au prochain démarrage.
  - [x] Créer puis supprimer build/emulation-session-start-check pour vérifier la destruction après un échec du démarrage et de l’arrêt, puis le démarrage d’un nouveau moteur.

- [x] Libération du moteur à l’extinction
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour partager la destruction entre extinction et échec de démarrage et préserver les médias au redémarrage.
  - [x] Créer puis supprimer build/emulation-power-off-check pour vérifier les deux chemins de l’extinction et le remontage au redémarrage.

- [x] Pistes magnétiques et mécanique des lecteurs dans le SDK
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h pour exposer la mécanique et les opérations de disque manquantes, en ajoutant les méthodes après les contrats existants.
  - [x] Modifier src/Emulation/src/MediaPreparation.h et src/Emulation/src/MediaPreparation.cpp pour relier ces opérations à MediaEngine et partager la mécanique entre le contrôleur et son image.
  - [x] Créer puis supprimer build/emulation-drive-sdk-check pour vérifier le SDK depuis un module natif, avec une image temporaire et un lecteur partagé.

- [x] Sortie hôte des bruitages mécaniques
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h et src/Emulation/src/MediaPreparation.h et .cpp pour laisser le module choisir le rendu PCM ou la sortie hôte.
  - [x] Modifier src/Emulation/src/AudioOutput.h et .cpp pour réveiller, mélanger et libérer la sortie des lecteurs selon le cycle du C#.
  - [x] Créer puis supprimer build/emulation-host-audio-check pour vérifier les événements transmis depuis un module et distinguer le rendu hôte du rendu PCM.

- [x] Préparation des médias dans le SDK
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h pour exposer les contrats de préparation des bandes, CUE, disques et sons mécaniques.
  - [x] Modifier src/Emulation/src/MediaPreparation.h et .cpp pour raccorder les méthodes au moteur média et préserver les métadonnées des blocs de bande.
  - [x] Créer puis supprimer build/emulation-media-preparation-check pour vérifier les CUE, les tailles de secteurs, les catalogues et la lecture et l’écriture d’une bande depuis un module natif.

- [x] Recréation du moteur après changement de média
  - [x] Modifier src/Emulation/src/Host.cpp pour détruire le moteur avant la fabrique et conserver le montage précédent en cas de refus ou d’annulation.
  - [x] Créer puis supprimer build/emulation-runtime-replacement-check pour exercer un moteur qui exige une ressource exclusive et une recréation en erreur.

- [x] Présentation des erreurs dans les réglages et les sessions
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour transmettre les erreurs structurées et utiliser le nom de la machine lorsque la configuration est anonyme.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationErrorDialog.cpp pour consigner la gravité et la cause technique des erreurs.
  - [x] Créer puis supprimer build/emulation-machine-error-check pour ouvrir le dialogue avec une machine de test et vérifier le nom de la machine.

- [x] Clavier et manettes avec l’OSD sans pause
  - [x] Modifier src/App/Shared/Emulation/EmulationPhysicalShortcuts.h pour identifier les commandes de manette consommées par un raccourci.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour transmettre les entrées de jeu pendant l’OSD sans pause et filtrer les commandes de navigation jusqu’au relâchement.
  - [x] Créer puis supprimer build/emulation-live-osd-input-check pour vérifier le clavier dans le thème Release et le masque des sources de manette.

- [x] Souris dans l’OSD sans pause
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour détecter les panneaux actifs et les contrôles qui capturent la souris.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour transmettre la souris au jeu hors des panneaux et neutraliser les boutons consommés jusqu’à leur relâchement.
  - [x] Créer puis supprimer build/emulation-live-osd-pointer-check pour vérifier les panneaux, le passage de la souris au jeu et le relâchement des boutons.

- [x] Informations machine dans le menu plein écran
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reprendre l’ordre des rubriques et regrouper les champs informatifs exposés par le module.
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour présenter les informations en lecture seule dans un panneau central défilant.
  - [x] Créer puis supprimer build/emulation-osd-information-check pour vérifier les informations du module, la visibilité des champs et le retour au menu.

- [x] Transparence du menu plein écran
  - [x] Modifier src/Common/Constants/DisplayConstants.h, src/Infrastructure/State/PreferencesStore.h et .cpp, src/App/State/WorkspaceState.h et .cpp et src/App/Bootstrap/Application.cpp pour enregistrer la transparence globale, 5 % par défaut et bornée de 0 à 100 %.
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et Presentation.h et les sources homonymes dans src/VideoPresentation/src pour appliquer le réglage à l’arrière-plan des panneaux et indicateurs plein écran.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et EmulationPreferencesDialog.cpp pour proposer le réglage dans l’OSD et les préférences.
  - [x] Modifier src/Translations/Catalogs/Constants/App/emulation-osd.json et les fichiers App/emulation-osd.json des langues correspondantes pour y reprendre le libellé du C#.
  - [x] Créer puis supprimer build/emulation-osd-transparency-check pour vérifier le réglage par défaut, l’aperçu et la relecture après écriture.

- [x] Volume de la machine en plein écran
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour appliquer le volume de session à la sortie audio, y compris après recréation du moteur.
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour afficher l’indicateur temporaire de volume et appliquer la transparence de l’OSD.
  - [x] Modifier src/App/Shared/Emulation/EmulationKeyboardShortcuts.h et src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et EmulationPreferencesDialog.cpp pour raccorder le volume dans le menu Audio et les deux raccourcis configurables.
  - [x] Modifier les catalogues App/emulation-shortcuts.json dans src/Translations/Catalogs pour ajouter les libellés repris du C#.
  - [x] Créer puis supprimer build/emulation-osd-volume-check pour vérifier le volume de session, le raccourci, la transparence et la disparition de l’indicateur.

- [x] Indicateur de sourdine en plein écran
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour distinguer un gain nul d’un faible volume arrondi à zéro.
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour afficher le pictogramme de sourdine en bas à droite selon FullscreenMuteOsd.cs.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour piloter l’indicateur avec le volume, la commande de sourdine et le cycle plein écran.
  - [x] Créer puis supprimer build/emulation-osd-mute-check pour vérifier la sourdine, le gain nul, le placement, la conservation du focus et la fermeture.

- [x] Mixeur audio dans le menu plein écran
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h et Host.h ainsi que src/Emulation/src/Host.cpp pour lire les groupes de mixage déclarés par le module.
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour présenter les curseurs verticaux, la navigation et la sauvegarde des valeurs.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour raccorder le volume général et les canaux du module dans le mixeur.
  - [x] Modifier les fichiers App/emulation-osd.json dans src/Translations/Catalogs pour réutiliser la traduction du mixeur de la branche C#.
  - [x] Créer puis supprimer build/emulation-osd-mixer-check pour vérifier le volume général, les canaux de module, la visibilité, la navigation et l’écriture.

- [x] Statistiques en plein écran
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour afficher les cinq lignes colorées du C# dans un panneau en haut à droite.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour alimenter le panneau avec le moteur réellement actif, la taille, les trames par seconde, la fréquence et le processeur.
  - [x] Créer puis supprimer build/emulation-osd-statistics-check pour vérifier le contenu du panneau, sa position, sa transparence et sa désactivation.

- [x] Temporisation et capture de la souris dans l’OSD
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour suspendre la fermeture pendant la capture et relancer le délai uniquement lors des interactions dans le menu.
  - [x] Créer puis supprimer build/emulation-osd-idle-check pour vérifier la capture dans l’éditeur, le mixeur, le clic hors menu et la fermeture automatique.

- [x] Canaux à choix discrets dans le mixeur plein écran
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour présenter les choix nommés de mixage en curseurs verticaux, comme le C#, et écrire leur identifiant.
  - [x] Créer puis supprimer build/emulation-osd-mixer-choices-check pour vérifier les libellés nommés, les bornes et l’écriture de l’identifiant choisi.

- [x] Navigation dans les listes en lecture seule de l’OSD
  - [x] Modifier src/VideoPresentation/src/FullscreenOsd.cpp pour parcourir à la manette ou au clavier les entrées visibles, y compris les informations et les commandes indisponibles, tout en bloquant leur activation.
  - [x] Créer puis supprimer build/emulation-osd-readonly-check pour vérifier la navigation sur les lignes informatives, le saut des séparateurs et le blocage des commandes indisponibles.

- [x] Raccourcis audio et répétition dans l’OSD
  - [x] Modifier src/VideoPresentation/src/FullscreenOsd.cpp pour traiter le mixeur comme un éditeur lors du maintien d’une direction.
  - [x] Modifier src/App/Shared/Emulation/EmulationKeyboardShortcuts.h et EmulationPhysicalShortcuts.h ainsi que src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reprendre les raccourcis audio actifs dans l’OSD et le délai de répétition du C#.
  - [x] Créer puis supprimer build/emulation-osd-repeat-check pour vérifier le maintien d’une direction dans le mixeur et la répétition du volume.

- [x] Portée des raccourcis pendant l’ouverture de l’OSD
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour rattacher au menu plein écran uniquement ses raccourcis, le volume et la sourdine.
  - [x] Créer puis supprimer build/emulation-osd-shortcut-focus-check pour vérifier que les commandes de session restent inactives pendant l’ouverture du menu et redeviennent accessibles après sa fermeture.

- [x] Restitution des entrées après un OSD avec pause
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour vider les touches ordinaires suivies par les raccourcis et mémoriser les boutons de souris encore enfoncés au retour à la machine.
  - [x] Créer puis supprimer build/emulation-osd-input-return-check pour vérifier le retour au jeu et la répétition du volume quand le jeu ne reçoit pas le relâchement de la touche.

- [x] Priorité des raccourcis de volume et du plein écran sur le jeu
  - [x] Modifier src/App/Shared/Emulation/EmulationKeyboardShortcuts.h et src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour retirer les touches des raccourcis de volume et de bascule OSD des entrées remises à la machine.
  - [x] Créer puis supprimer build/emulation-osd-shortcut-input-check pour vérifier les entrées du jeu, le volume et l’ouverture du menu sans pause.

- [x] Alias de commandes des manettes dans les raccourcis
  - [x] Modifier src/App/Shared/Emulation/EmulationPhysicalShortcuts.h pour reconnaître les noms de boutons et de gâchettes également acceptés par le C# et les filtrer de la même façon que les noms capturés par le C++.
  - [x] Créer puis supprimer build/emulation-gamepad-alias-check pour vérifier la pression, le filtrage et le ciblage avec des états de manette contrôlés.

- [x] Capture des axes analogiques de manette
  - [x] Modifier src/App/Shared/Emulation/EmulationPhysicalShortcuts.h pour mesurer le mouvement brut depuis la position initiale comme dans le C#.
  - [x] Modifier src/Platform/include/RetroMediaLab/Platform/Gamepads.h pour reconnaître les axes bruts numérotés indépendamment de la casse dans la capture et la reprise des entrées.
  - [x] Créer puis supprimer build/emulation-gamepad-axis-check pour vérifier les mouvements depuis une position décentrée, la direction et le seuil de reprise.

- [x] Boutons système des manettes Windows
  - [x] Modifier src/Platform/Windows/Gamepads.cpp et src/Platform/include/RetroMediaLab/Platform/Gamepads.h pour publier les états Guide et Share reçus de GameInput et libérer les deux callbacks à la fermeture.
  - [x] Modifier src/App/Shared/Emulation/EmulationPhysicalShortcuts.h pour reconnaître Guide comme alias de XboxButton dans le filtre des contrôles bruts.
  - [x] Créer puis supprimer build/emulation-gamepad-system-check pour vérifier les états transmis à la capture et au filtre de raccourcis.

- [x] Boutons système dans le mappage des machines
  - [x] Modifier src/Emulation/src/InputProcessing.cpp pour interpréter les contrôles nommés XboxButton et Share reçus du backend dans les mappages et combinaisons du SDK.
  - [x] Créer puis supprimer build/emulation-system-mapping-check pour vérifier les commandes simples, les combinaisons et les états encodés en bits.

- [x] Système de fichiers virtuel par défaut pour les cœurs natifs
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreEnvironment.h et src/Emulation/src/NativeCoreEnvironment.cpp pour fournir un VFS détenu par la session quand aucun fournisseur personnalisé ne lui est prêté et le libérer après la désinitialisation du cœur.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreFileSystem.h et src/Emulation/src/NativeCoreFileSystem.cpp pour partager la version de négociation avant allocation.
  - [x] Créer puis supprimer build/emulation-native-default-vfs-check pour vérifier la lecture par un cœur natif et la libération à sa fermeture.

- [x] Modes d’accès du VFS des cœurs natifs
  - [x] Modifier src/Emulation/src/NativeCoreFileSystem.cpp pour ouvrir les fichiers avec le mode exact, éviter la troncature en mise à jour et autoriser le partage de suppression sous Windows comme en C#.
  - [x] Créer puis supprimer build/emulation-native-vfs-access-check pour vérifier les accès, la conservation des données et le renommage avec un fichier ouvert.

- [x] Chemins longs du VFS des cœurs natifs
  - [x] Modifier src/Emulation/src/NativeCoreFileSystem.cpp pour ouvrir les chemins absolus longs sous Windows et conserver le chemin ordinaire exposé au cœur.
  - [x] Créer puis supprimer build/emulation-native-vfs-long-path-check pour vérifier ouverture, écriture, relecture et chemin exposé au-delà de MAX_PATH.

- [x] Fermeture de la préparation OpenGL
  - [x] Modifier src/VideoPresentation/src/OpenGlSurface.h et src/VideoPresentation/src/OpenGlSurface.cpp pour que la surface possède et libère la tâche de compilation avant le contexte.
  - [x] Créer puis supprimer build/emulation-opengl-lifecycle-check pour vérifier la préparation, le changement de moteur et la fermeture de la vue.

- [x] Présentation des réglages de stockage d’un module
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et EmulationStoragePanel.h pour transmettre les clés d’aide détaillée déclarées par le module.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationStoragePanel.cpp pour utiliser les aides du thème et présenter les valeurs informatives avec les libellés du module.

- [x] Saisie IPv4 des réglages de machine
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h pour déclarer l’éditeur IPv4.
  - [x] Créer src/App/Shared/Controls/ModuleIpv4Edit.h et modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp, RetroMediaLab.vcxproj et RetroMediaLab.vcxproj.filters pour raccorder l’éditeur au dialogue de la machine.
  - [x] Créer puis supprimer build/emulation-ipv4-edit-check pour vérifier la saisie, le collage et la conservation de la dernière adresse complète.

- [x] Ouverture de dossier dans les réglages de machine
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h pour définir l’action et son chemin absolu fourni par le module.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour créer et ouvrir ce dossier par le gestionnaire de fichiers et afficher une erreur localisée en cas d’échec.
  - [x] Modifier src/Translations/Catalogs/Constants/App/emulation-folders.json et les catalogues Languages correspondants pour reprendre le texte d’action de la branche C#.

- [x] Indication de préparation vidéo lors des changements de vue
  - [x] Modifier src/VideoPresentation/src/Presentation.cpp pour conserver le chargement jusqu’à la réponse du moteur lors du masquage et de l’effacement de l’image.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour masquer et réafficher l’indicateur au changement de visibilité de la surface.
  - [x] Créer puis supprimer build/emulation-shader-visibility-check pour vérifier la préparation pendant le masquage, l’effacement et le retour à la vue.

- [x] Réinitialisation des entrées lors d’une modification de session
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour vider les entrées avant la première lecture du nouveau moteur et après la sauvegarde des affectations.

- [x] Restauration des médias à la première mise sous tension
  - [x] Modifier src/Emulation/src/Host.cpp pour partager les montages de la configuration entre l’ouverture de la session, la première mise sous tension et la recréation du moteur.
  - [x] Créer puis supprimer build/emulation-initial-mounts-check pour exercer le premier démarrage et le redémarrage avec un module temporaire.

- [x] Transmission des médias à la fabrique de machines comme dans le C#
  - [x] Modifier src/Emulation/src/Host.cpp pour transmettre les montages à createRuntimeWithMedia dès l’ouverture et à la recréation quand le module fournit les deux contrats.
  - [x] Créer puis supprimer build/emulation-mount-factory-check pour vérifier la construction initiale et la recréation sans insertion tardive.

- [x] Transmission du moteur vidéo au module
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h, Host.h et src/Emulation/src/Host.cpp pour appliquer l’API graphique à la configuration d’exécution et aux options à chaud.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour transmettre le moteur enregistré à l’ouverture et après un changement dans le dialogue ou l’OSD.
  - [x] Créer puis supprimer build/emulation-graphics-api-check pour vérifier la configuration transmise à la fabrique, les options à chaud et la recréation.

- [x] Prise en compte du moteur vidéo dans la description des réglages
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour exposer la préparation vidéo déjà utilisée à l’exécution.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour décrire les options avec le moteur du profil dans l’éditeur et l’OSD.

- [x] Règles des réglages modifiés depuis la session
  - [x] Modifier src/Emulation/src/Host.cpp pour calculer la modification et ses dépendances avec le même moteur vidéo que celui de l’éditeur de session.

- [x] Choix du moteur vidéo dans les réglages de machine
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour proposer le moteur dans l’onglet Vidéo, conserver les modifications, enregistrer le profil et reconstruire les réglages en restant sur Vidéo.

- [x] Visibilité et aide du sélecteur vidéo de machine
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour respecter la visibilité de l’onglet déclarée par le module et reprendre l’aide du sélecteur de moteur du C#.
  - [x] Modifier src/Translations/Catalogs/Constants/App/emulation-video-processing.json et les catalogues de langues correspondants avec les deux aides du C#.

- [x] Présentation du matériel vidéo dans les réglages de machine
  - [x] Modifier src/App/Shared/Emulation/EmulationWorkspace.h et EmulationWorkspace.cpp pour appliquer aux réglages Mémoire les critères du C# sur le matériel vidéo.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour présenter ces réglages dans Vidéo lorsque ce volet est visible.

- [x] Volet Matériel vidéo des réglages de machine
  - [x] Modifier src/App/Shared/Emulation/EmulationWorkspace.h et EmulationWorkspace.cpp pour identifier aussi les jeux de puces vidéo du C# et conserver les dépendances de leurs groupes.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour afficher le volet matériel à droite du rendu.
  - [x] Modifier les catalogues App/emulation-video-processing.json avec le titre du volet repris du C#.

- [x] Réglages de traitement vidéo avant la mise sous tension
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour réutiliser les contrôles vidéo existants depuis les réglages de machine et sauvegarder le profil avec ou sans session active.

- [x] Onglets Affichage et Image des réglages de traitement
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour placer le choix de technologie dans Affichage et les paramètres d’image et de technologie dans Image, avec défilement vertical.
  - [x] Modifier les catalogues App/emulation-video-processing.json avec les deux libellés du C#.

- [x] Sauvegarde du traitement vidéo dans l’éditeur de machine
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour enregistrer les paramètres de traitement sur place et réserver la reconstruction de l’éditeur aux changements de moteur.

- [x] Actualisation des montages après modification du stockage
  - [x] Modifier src/Emulation/src/Host.cpp pour actualiser les médias de la session depuis les emplacements configurés après la sauvegarde.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour rafraîchir la barre des médias et les réglages OSD après une sauvegarde.

- [x] Vérification de la sauvegarde du stockage et du redémarrage
  - [x] Créer puis supprimer build/emulation-storage-refresh-check/check.cpp, sa DLL et son exécutable pour vérifier le changement de montage via le SDK et le moteur recréé.

- [x] Affichage des lecteurs configurés dans la barre des médias
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour exposer les lecteurs de la configuration aux commandes de session.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour utiliser la liste de la session dans la barre des médias.
  - [x] Créer puis supprimer build/emulation-storage-slots-check pour vérifier le changement de lecteurs avant le redémarrage.

- [x] Synchronisation des montages lors des changements de configuration
  - [x] Modifier src/Emulation/src/Host.cpp pour recalculer les montages lorsqu’une option de machine ou le moteur vidéo fait évoluer les lecteurs.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour actualiser la barre après le changement de moteur vidéo.
  - [x] Créer puis supprimer build/emulation-storage-configuration-check pour vérifier les deux changements et le redémarrage.

- [x] Description et validation communes des lecteurs
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour utiliser la configuration préparée pour le moteur vidéo dans la description et la validation des lecteurs.
  - [x] Modifier src/App/Shared/Emulation/EmulationWorkspace.h, EmulationWorkspace.cpp et src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour transmettre le moteur du profil et reconstruire la barre lorsque les propriétés du lecteur changent.
  - [x] Créer puis supprimer build/emulation-storage-description-check pour vérifier la description et la validation des lecteurs avec et sans session.

- [x] Validation du lecteur et du transport avant l’enregistrement d’une cassette
  - [x] Modifier src/Emulation/src/Host.cpp pour vérifier le lecteur configuré, la mise sous tension et la commande d’enregistrement avant de créer le fichier.

- [x] Réutilisation de la description des lecteurs pendant le rafraîchissement
  - [x] Modifier src/Emulation/src/Host.cpp pour garder la description du stockage jusqu’au prochain changement de configuration, de moteur vidéo ou de machine d’exécution.
  - [x] Créer puis supprimer build/emulation-storage-cache-check pour vérifier la réutilisation et les trois invalidations de la description.

- [x] Vérification de la barre des médias et du dialogue dans le thème
  - [x] Créer puis supprimer build/emulation-video-storage-view-check pour charger le thème Release et exercer l’actualisation des lecteurs, le matériel vidéo et les onglets de traitement.

- [x] Suivi des boutons de souris pendant le menu plein écran sans pause
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour filtrer les boutons au relevé des entrées lorsque les panneaux bougent sous le pointeur, avec le même masque que les événements de souris.

- [x] Aligner la présentation vidéo et les nouvelles touches sur osd-support
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/Presentation.h, src/VideoPresentation/src/Presentation.cpp et SoftwareProcessing.cpp pour retirer les effets stylistiques du profil et du rendu.
  - [x] Supprimer src/VideoPresentation/src/StylisticProcessing.cpp et StylisticProcessing.h, retirer leur entrée de src/VideoPresentation/CMakeLists.txt et retirer les contrôles du dialogue et de l’OSD dans src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp.
  - [x] Supprimer les catalogues emulation-video-stylistic.json de src/Translations/Catalogs/Constants/App et src/Translations/Catalogs/Languages/*/App.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/EmulationKey.h pour reprendre les deux identifiants Sega communs.
  - [x] Créer puis supprimer build/osd-support-profile-check pour vérifier le chargement et la réécriture d’un ancien profil sans effets stylistiques.

- [x] Relayer les affectations clavier déclarées par la machine
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/InputSettings.h pour exposer les associations clavier de la session par contrat facultatif.
  - [x] Modifier src/App/Shared/Emulation/EmulationHostKeyboard.h et EmulationHostKeyboard.cpp pour séparer les caractères hôte des commandes clavier et évaluer les sources composites avec les règles communes.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour transmettre les boutons, molettes et manettes filtrés et donner priorité aux raccourcis de l’application.
  - [x] Créer puis supprimer build/emulation-keyboard-bindings-check pour vérifier les sources physiques, la priorité des raccourcis et les caractères du clavier.

- [x] Vérifier la publication des affectations depuis la vue Émulation
  - [x] Créer puis supprimer build/emulation-keyboard-bindings-view-check pour vérifier la combinaison clavier et molette, le relâchement et la priorité des raccourcis dans le thème Release.

- [x] Reprendre les opérations communes d’installation des cœurs
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/CoreReleases.h et src/Emulation/src/CoreReleases.cpp pour construire les chemins, extraire la bibliothèque attendue, remplacer le fichier et résoudre le manifeste de la version active.
  - [x] Modifier src/Emulation/CMakeLists.txt pour compiler ces opérations et utiliser le libarchive du projet.
  - [x] Créer puis supprimer build/emulation-core-installation-check pour vérifier le contenu extrait, la conservation de la destination en cas d’échec et la sélection d’une installation complète.

- [x] Métadonnées et téléchargement des versions de cœurs
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/CoreReleases.h et src/Emulation/src/CoreReleases.cpp pour reprendre la sélection du catalogue, les métadonnées HTTP, le téléchargement avec progression et l’ordre d’installation.
  - [x] Créer puis supprimer build/emulation-core-download-check pour vérifier les réponses HTTP locales, l’écriture du fichier et l’annulation ainsi que l’ordre des étapes d’installation.

- [x] Contexte OpenGL des cœurs sur Windows
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/NativeCoreHardwareRender.h et src/Emulation/src/NativeCoreHardwareRender.cpp pour exposer le contexte, ses callbacks et la copie des trames.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreEnvironment.h et src/Emulation/CMakeLists.txt pour déclarer la commande native et lier le backend Windows.
  - [x] Créer puis supprimer build/emulation-core-opengl-check pour vérifier les callbacks, les pixels, le redimensionnement et le nettoyage du contexte.

- [x] Tampons vidéo réutilisables du SDK
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreFrames.h et src/Emulation/src/NativeCoreFrames.cpp pour porter le calcul de taille, la copie des pixels et la rotation des tampons, puis les raccorder au callback vidéo natif.
  - [x] Créer puis supprimer build/emulation-video-buffers-check pour vérifier la rotation, la conservation des trames publiées et les dimensions invalides.

- [x] Contrôleur de sortie audio du SDK
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/AudioOutputController.h et src/Emulation/src/AudioOutputController.cpp pour porter le cycle de vie, le volume, la latence et la récupération de la sortie audio du C#.
  - [x] Modifier src/Emulation/CMakeLists.txt et actualiser les fichiers Visual C++ générés pour exposer le contrôleur.
  - [x] Créer puis supprimer build/emulation-audio-controller-check pour vérifier le comportement avec des sorties empruntées et possédées.

- [x] Géométrie et libellés vidéo du SDK
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreFrames.h et src/Emulation/src/NativeCoreFrames.cpp pour appliquer le rapport d’image et produire les libellés de fréquence et de norme comme dans le C#.
  - [x] Modifier src/Emulation/src/NativeCoreEnvironment.cpp pour utiliser la même règle de géométrie à l’initialisation et pendant l’émulation.
  - [x] Créer puis supprimer build/emulation-video-geometry-check pour vérifier les deux chemins et les libellés.

- [x] Cache de pipelines Vulkan entre lancements
  - [x] Créer src/VideoPresentation/src/ShaderDiskCache.h à partir des opérations de lecture et d’écriture Direct3D 11.
  - [x] Modifier src/VideoPresentation/src/Direct3DSurface.cpp et src/VideoPresentation/src/VulkanSurface.cpp pour partager le stockage et réutiliser le cache Vulkan sur le fil de préparation.
  - [x] Mettre à jour src/VideoPresentation/CMakeLists.txt et les fichiers Visual C++ générés pour inclure le code commun.
  - [x] Créer puis supprimer build/emulation-vulkan-disk-cache-check pour vérifier la persistance, la lecture et la reconstruction après corruption.

- [x] Utilitaires de traitement audio du SDK
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/AudioProcessing.h et src/Emulation/src/AudioProcessing.cpp pour porter les règles audio de configuration, de copie, de troncature et de volume.
  - [x] Modifier src/Emulation/src/AudioBuffer.cpp, src/Emulation/src/AudioOutputController.cpp et src/Emulation/CMakeLists.txt pour réutiliser ces règles, puis actualiser les fichiers Visual C++.
  - [x] Créer puis supprimer build/emulation-audio-processing-check pour vérifier les utilitaires et leur raccordement au tampon audio.

- [x] En-têtes des panneaux centraux de l’OSD
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationCommandIcons.h et src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reprendre les glyphes et l’ordre des sections du C#.
  - [x] Modifier src/VideoPresentation/src/FullscreenOsd.cpp pour afficher l’icône du panneau et le séparateur dans Informations et Mixeur.
  - [x] Créer puis supprimer build/emulation-osd-panel-header-check pour vérifier les en-têtes et les glyphes.

- [x] Commandes affichées dans les panneaux centraux de l’OSD
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour afficher les raccourcis actifs et offrir le retour à la souris dans Informations et Mixeur.
  - [x] Créer puis supprimer build/emulation-osd-center-footer-check pour vérifier les deux panneaux, l’actualisation des raccourcis et le retour.

- [x] Application des réglages du mixeur plein écran
  - [x] Modifier src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h et src/VideoPresentation/src/FullscreenOsd.cpp pour appliquer les changements pendant le déplacement et regrouper les écritures en attente par voie, comme dans le C#.
  - [x] Créer puis supprimer build/emulation-osd-mixer-save-check pour vérifier le déplacement, la conservation du dernier réglage de chaque voie et l’écriture au retour.

- [x] Sélection des voies à la souris dans le mixeur
  - [x] Modifier src/VideoPresentation/src/FullscreenOsd.cpp pour sélectionner la voie depuis sa carte et déplacer directement le curseur au point cliqué.
  - [x] Créer puis supprimer build/emulation-osd-fader-pointer-check pour vérifier la sélection depuis le libellé et le réglage sur la piste du curseur.

- [x] Diagnostic des bibliothèques de cœur avant chargement
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/CoreDiagnostics.h et src/Emulation/src/CoreDiagnostics.cpp pour produire le manifeste du C#, vérifier l’architecture et valider une installation en lisant les métadonnées PE.
  - [x] Modifier src/Emulation/CMakeLists.txt pour lier la lecture Windows des ressources de version.
  - [x] Créer puis supprimer build/emulation-core-diagnostic-check pour vérifier une DLL Release et les erreurs de vérification.

- [x] Règles des médias du SDK osd-support
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/StorageSettings.h et src/Emulation/src/StorageSettings.cpp pour exposer la validation, le remplacement, l’éjection et le protocole JSON des montages.
  - [x] Modifier src/Emulation/src/Host.cpp pour appliquer ces règles aux montages persistés.
  - [x] Créer puis supprimer les fichiers sous build/emulation-media-rules-check pour vérifier les règles et le protocole JSON.

- [x] Empreintes de contenu des sauvegardes
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/StateFile.h et src/Emulation/src/StateFile.cpp pour calculer les empreintes des flux et arborescences de contenu selon osd-support.
  - [x] Créer puis supprimer build/emulation-state-content-check pour comparer les empreintes de fichiers, flux et dossiers.

- [x] Entrées-sorties des sauvegardes selon osd-support
  - [x] Modifier src/Emulation/src/StateFile.cpp pour réessayer les erreurs de lecture, conserver les erreurs de format et créer le dossier avant l’écriture atomique.
  - [x] Créer puis supprimer build/emulation-state-io-check pour vérifier les trois catégories de lecture ainsi que l’écriture dans un nouveau dossier.

- [x] Empreinte canonique de configuration des sauvegardes
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/StateFile.h et src/Emulation/src/StateFile.cpp pour hacher les propriétés JSON dans l’ordre ordinal, sans modifier l’ordre des tableaux.
  - [x] Créer puis supprimer build/emulation-state-document-check pour comparer les empreintes du SDK au résultat de System.Text.Json.

- [x] Journal des cœurs natifs
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreEnvironment.h et src/Emulation/src/NativeCoreEnvironment.cpp pour porter le traitement des formats de journal de osd-support.
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreSession.h et src/Emulation/src/NativeCoreSession.cpp pour fournir et lire les diagnostics du journal par défaut.
  - [x] Créer puis supprimer build/emulation-native-log-check pour vérifier le trajet du rappel derrière une DLL temporaire.

- [x] Témoins du cœur natif
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreSession.h et src/Emulation/src/NativeCoreSession.cpp pour recevoir les états LED et les exposer au module.
  - [x] Créer puis supprimer build/emulation-native-led-check pour vérifier les états du rappel par défaut et la priorité du rappel fourni par le module.

- [x] Mémorisation des dossiers dans les réglages d’émulation
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour mémoriser les dossiers par champ de machine tout en conservant le chemin du brouillon.

- [x] Rafraîchissement des réglages de machine selon osd-support
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/{Sdk,Host}.h et src/Emulation/src/Host.cpp pour exposer les champs qui demandent une nouvelle description.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour redécrire le formulaire depuis le brouillon et reconstruire les champs dans le dialogue ouvert.

- [x] Validation du formulaire de réglages dépendants
  - [x] Créer puis supprimer build/emulation-settings-refresh-check pour ouvrir les options du thème avec un module temporaire, contrôler les champs redécrits et vérifier la sauvegarde du brouillon.

- [x] Présentation des choix de machine selon osd-support
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h, src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour publier les choix radio et les clés d’unité.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/ModuleSettingPresentation.h, EmulationPage.cpp et EmulationStoragePanel.{h,cpp} pour afficher les choix et les enregistrer selon la description du module.
  - [x] Créer puis supprimer build/emulation-radio-check pour vérifier la sélection, les unités et les notifications des boutons radio.

- [x] Vitesses génériques des lecteurs de disquettes
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/StorageSettings.h et src/Emulation/src/StorageSettings.cpp pour exposer les préréglages de EmulationDriveSpeedChoices.cs.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationStoragePanel.cpp pour utiliser les vitesses génériques quand le module omet la liste et conserver les choix explicites.
  - [x] Modifier les catalogues App/emulation-storage.json de Constants et des langues sous src/Translations/Catalogs avec la clé Emulation.Value.Maximum issue de EmulationValue.resx.

- [x] Focus pendant l’affichage du volume en plein écran
  - [x] Créer puis supprimer build/emulation-volume-focus-check pour vérifier le focus du menu lors de l’apparition et de la disparition de l’indicateur de volume.
  - [x] Modifier src/VideoPresentation/src/FullscreenOsd.cpp pour afficher le volume sans prendre le focus du menu ou de la machine.

- [x] Validation Windows native du focus de l’indicateur de volume
  - [x] Créer puis supprimer build/emulation-volume-focus-check pour contrôler la fenêtre active avant, pendant et après le volume avec le plugin Windows Qt.

- [x] Molette dans l’éditeur de l’OSD
  - [x] Modifier src/VideoPresentation/src/FullscreenOsd.cpp pour appliquer un seul cran sur tout le panneau de réglage, y compris son curseur.
  - [x] Créer puis supprimer build/emulation-osd-wheel-check pour vérifier le panneau, le curseur et la sauvegarde.

- [x] Unités des réglages de module dans l’OSD
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reprendre les clés d’unité fournies par le module dans les éditeurs, le mixeur et les lignes d’information de l’OSD.

- [x] Libellés des curseurs numériques du dialogue machine
  - [x] Modifier src/App/Shared/Emulation/ModuleNumericSetting.h pour formater le libellé du curseur standard avec huit décimales facultatives, son unité localisée et son suffixe.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour appliquer le libellé et la clé déclarée par le module.
  - [x] Créer puis supprimer build/emulation-slider-label-check pour vérifier la précision, le suffixe et la valeur automatique.

- [x] Routage des sources physiques qualifiées
  - [x] Modifier src/Emulation/src/InputProcessing.cpp pour refuser les préfixes inconnus et les identifiants de manette vides dans la validation et le calcul de pression.
  - [x] Créer puis supprimer build/emulation-qualified-input-check pour contrôler les sources génériques, explicites, absentes et invalides.

- [x] Publication des sauvegardes numérotées
  - [x] Créer src/Emulation/src/StagedFile.h pour mutualiser le remplacement atomique déjà utilisé lors de l’installation des cœurs.
  - [x] Modifier src/Emulation/src/Host.cpp et src/Emulation/src/CoreReleases.cpp pour publier leurs fichiers préparés avec ce remplacement.
  - [x] Créer puis supprimer build/emulation-staged-file-check pour vérifier la création, le remplacement et la conservation de la destination après échec.

- [x] Enregistrement asynchrone des captures machine
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour encoder les instantanés en arrière-plan et publier le résultat sur le fil de l’interface.
  - [x] Modifier src/App/Shared/Emulation/EmulationWorkspace.cpp pour attribuer des noms distincts aux captures concurrentes.
  - [x] Créer puis supprimer build/emulation-concurrent-capture-check pour vérifier les PNG écrits et leurs noms.

- [x] Message en absence de trame pour une capture machine
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour afficher la raison du refus de capture de MachineController.CaptureScreenAsync.
  - [x] Modifier App/emulation-shortcuts.json dans Constants et chaque langue pour reprendre Emulation.Shortcut.Unavailable de osd-support.

- [x] Validation de la capture depuis une session
  - [x] Créer puis supprimer build/emulation-session-capture-check pour exercer la commande configurée dans le thème avec et sans image.

- [x] Dialogue d’erreur des captures de session
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour présenter les échecs de capture dans le dialogue d’erreur de la machine et garder le nom du PNG après réussite.
  - [x] Créer puis supprimer build/emulation-session-capture-check pour vérifier le refus sans trame et l’écriture avec trame.

- [x] Menus de sauvegarde et de chargement d’une machine
  - [x] Créer puis supprimer build/emulation-state-menu-check pour vérifier les dix choix des deux menus du thème, puis écrire et recharger le dernier emplacement.

- [x] Emplacements de sauvegarde dans le menu plein écran
  - [x] Modifier src/VideoPresentation/src/FullscreenOsd.cpp pour garder le numéro seul lorsqu’un emplacement n’a pas de date.
  - [x] Créer puis supprimer build/emulation-osd-state-check pour ouvrir les listes de sauvegarde et de chargement du menu plein écran et exercer l’emplacement 10.

- [x] Descripteurs natifs avec limites choisies par le module
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreEnvironment.h et src/Emulation/src/NativeCoreEnvironment.cpp pour exposer les copies bornées des entrées, ports et cartes mémoire, avec les deux politiques de copie des ports du C#.
  - [x] Créer puis supprimer build/emulation-native-descriptor-check pour vérifier les limites, le rejet du dépassement, les noms vides et la copie des données.

- [x] Langue d’interface des cœurs natifs
  - [x] Modifier src/Emulation/src/NativeCoreEnvironment.cpp pour reprendre la table de langues de osd-support pour GetLanguage.
  - [x] Créer puis supprimer build/emulation-native-language-check pour vérifier les codes renvoyés, les variantes et le repli anglais.

- [x] Validation des options modernes au démarrage
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreOptions.h et src/Emulation/src/NativeCoreOptions.cpp pour vérifier après déclaration les valeurs configurées et lever l’erreur OptionInvalid.
  - [x] Modifier src/Emulation/src/NativeCoreSession.cpp pour valider les valeurs après l’initialisation native et avant le chargement du contenu.
  - [x] Créer puis supprimer build/emulation-native-option-validation-check pour exercer la validation et le parcours de démarrage.

- [x] Erreurs des valeurs natives anciennes et modifiées à chaud
  - [x] Modifier src/Emulation/src/NativeCoreOptions.cpp pour partager l’erreur OptionInvalid entre l’enregistrement ancien, le réglage côté hôte et la validation du démarrage.
  - [x] Créer puis supprimer build/emulation-native-option-errors-check pour vérifier le code traduit, le diagnostic, la conservation de la valeur et le refus côté cœur.

- [x] Cadence vidéo et fréquence audio annoncées par le cœur
  - [x] Modifier src/Emulation/src/NativeCoreEnvironment.cpp pour appliquer la géométrie et conserver les dernières fréquences valides selon ApplySystemAvInfo de osd-support.
  - [x] Créer puis supprimer build/emulation-native-av-timing-check pour vérifier la géométrie, les deux fréquences et leur mise à jour indépendante.

- [x] Libération de la latence et des chaînes d’options natives
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/NativeCoreOptions.h et src/Emulation/src/NativeCoreOptions.cpp pour libérer les chaînes empruntées et le rappel d’affichage à la fermeture.
  - [x] Modifier src/Emulation/src/NativeCoreEnvironment.cpp pour appeler ce nettoyage après la désinitialisation et remettre la latence à zéro.
  - [x] Créer puis supprimer build/emulation-native-environment-close-check pour vérifier leur disponibilité pendant la désinitialisation et le nettoyage ensuite.

- [x] Diagnostics des messages natifs historiques
  - [x] Modifier src/Emulation/src/NativeCoreEnvironment.cpp pour conserver les messages non vides, y compris les espaces seuls, selon SetMessage dans osd-support.

- [x] Nettoyage des diagnostics à la fermeture du cœur
  - [x] Modifier src/Emulation/src/NativeCoreEnvironment.cpp pour vider les diagnostics, les messages et l’erreur mémorisée après la désinitialisation.
  - [x] Créer puis supprimer build/emulation-native-diagnostics-close-check pour vérifier les messages de la session avant et après la fermeture.

- [x] Ordre de montage après remplacement d’un média
  - [x] Modifier src/Emulation/src/Host.cpp pour déplacer en fin de liste le média remplacé, comme MachineSession.InsertAsync.

- [x] Mise sous tension d’une session
  - [x] Modifier src/Emulation/src/Host.cpp pour ignorer Start sur une machine déjà sous tension, comme MachineSession.PowerOnAsync, et nettoyer un premier démarrage interrompu.
  - [x] Créer puis supprimer build/emulation-power-on-check pour contrôler la double mise sous tension, l’annulation et la remise en route avec un module temporaire.

- [x] Réglages image du mode Vectoriel
  - [x] Modifier src/VideoPresentation/src/Presentation.cpp pour exposer les cinq réglages du mode Vectoriel et les transmettre au rendu, comme sur osd-support.
  - [x] Créer puis supprimer build/emulation-vector-adjustments-check pour vérifier la disponibilité des commandes et la réponse des pixels.

- [x] Vérification matérielle des réglages vectoriels
  - [x] Créer puis supprimer build/emulation-vector-gpu-adjustments-check pour vérifier les cinq réglages sur Direct3D 11, Vulkan et OpenGL.

- [x] Courbure et netteté du rendu vectoriel matériel
  - [x] Modifier src/VideoPresentation/shaders/vector-raster.glsl pour courber le point de sortie avant le filtre de netteté, comme sur osd-support.
  - [x] Créer puis supprimer build/emulation-vector-edge-check pour vérifier la conservation d’une image uniforme jusqu’aux bords et le masque de courbure.

- [x] Glyphes des commandes du menu plein écran
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reprendre les glyphes de MachineCommandGlyphConstants sur le menu, les sauvegardes et le volume.
  - [x] Créer puis supprimer build/emulation-osd-command-glyphs-check pour contrôler les icônes du menu plein écran dans le thème chargé.

- [x] Ordre des réglages vidéo du menu plein écran
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour reprendre l’ordre de MachineController.OsdVideo, avec les réglages généraux, les contrôles du type d’écran et l’échantillonnage.
  - [x] Modifier src/Translations/Catalogs/{Constants,Languages/*}/App/emulation-video-processing.json pour reprendre le titre des réglages généraux dans les catalogues de la branche C#.
  - [x] Créer puis supprimer build/emulation-osd-video-order-check pour vérifier l’ordre des commandes avec le thème chargé.

- [x] Couleurs des rubriques du menu vidéo plein écran
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp et src/VideoPresentation/include/RetroMediaLab/VideoPresentation/FullscreenOsd.h pour signaler les titres de rubriques vidéo.
  - [x] Modifier src/VideoPresentation/src/FullscreenOsd.cpp pour reprendre les fonds du sous-menu et de l’éditeur de osd-support et les actualiser avec la sélection et la transparence.
  - [x] Créer puis supprimer build/emulation-osd-section-colors-check pour vérifier le changement de rubrique dans le thème chargé.

- [x] Contrat des adaptateurs de moteurs
  - [x] Créer src/Emulation/include/RetroMediaLab/Emulation/EmulatorAdapter.h pour reprendre les opérations et les contextes de IEmulationEmulatorAdapter, ainsi que la construction des fabriques du module.
  - [x] Modifier src/Emulation/CMakeLists.txt pour déclarer le contrat dans la cible Emulation.
  - [x] Créer puis supprimer build/emulation-adapter-contract-check pour vérifier le contrat et la découverte avec des fabriques temporaires.

- [x] Catalogues de moteurs et de modèles du SDK
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/EmulatorAdapter.h pour projeter les entrées de catalogue, retrouver un modèle par son identifiant exact et exposer la construction par contrat unique.
  - [x] Créer puis supprimer build/emulation-adapter-catalog-check pour vérifier les identifiants, la clé de description et la construction des adaptateurs.

- [x] Transmission des commandes aux cœurs hébergés au démarrage
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Sdk.h et Host.h ainsi que src/Emulation/src/Host.cpp pour exposer le contrat facultatif et retourner le premier code de sortie fourni par un module.
  - [x] Modifier src/App/Bootstrap/Application.cpp pour transmettre les arguments aux modules chargés avant le thème et les préférences, puis quitter si un module les traite.
  - [x] Créer puis supprimer build/emulation-host-command-check pour vérifier le lancement de l’application avec des modules temporaires.

- [x] Diagnostics de découverte des modules
  - [x] Modifier src/Emulation/include/RetroMediaLab/Emulation/Host.h et src/Emulation/src/Host.cpp pour signaler les DLL hors paquet et les dossiers sans manifeste.
  - [x] Modifier src/App/Shared/Emulation/EmulationWorkspace.h, src/App/Bootstrap/Application.cpp et src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour partager le message localisé et journaliser les diagnostics au démarrage.
  - [x] Créer puis supprimer build/emulation-module-diagnostics-check pour vérifier le rejet des paquets invalides et le rafraîchissement des diagnostics.

- [x] Distribution autonome du SDK d’émulation
  - [x] Modifier src/Emulation/CMakeLists.txt et créer src/Emulation/RetroMediaLabEmulationConfig.cmake.in pour installer le SDK et sa cible importée avec CMake.
  - [x] Créer puis supprimer build/emulation-sdk-check pour compiler un module à partir du SDK installé et vérifier son chargement par RML.
