# Séparer le socle App et le thème RetroMediaLab

- [x] Bibliothèque App et présentation
  - [x] Déplacer ThemeService.h, ThemeService.cpp et IThemePresentation.h de src/App/Themes vers src/App/Shared/Presentation, puis modifier les inclusions.
  - [x] Déplacer EmulationHostKeyboard.h/.cpp, EmulationKeyboardShortcuts.h, EmulationPhysicalShortcuts.h et ModuleNumericSetting.h vers src/App/Shared/Emulation.
  - [x] Déplacer ControllerArtwork.h/.cpp, ControllerOutputs.h/.cpp, MachineInputBindings.h/.cpp, EmulationShortcutCapture.h, ModuleEditableSlider.h, FlowLayout.h/.cpp et ContentScrollArea.h/.cpp vers src/App/Shared/Controls.
  - [x] Créer src/App/Shared/Presentation/DisplayFactor.h et modifier DisplayDensity.cpp et ContentScrollArea.cpp pour utiliser le facteur commun.
- [x] Assets externes
  - [x] Déplacer Controllers et FileTypes de src/App/Themes/RetroMediaLab vers src/App/Assets et icons.json vers src/App/Assets/Themes/RetroMediaLab.
  - [x] Déplacer metrics.json, light.json et dark.json dans src/App/Assets/Themes/RetroMediaLab et modifier RetroMediaLabConfiguration.cpp, ThemeConstants.h et src/App/CMakeLists.txt pour lire ces fichiers externes.
  - [x] Créer src/App/Shared/Presentation/AssetPath.h et modifier ControllerArtwork.cpp, ExplorerPage.cpp, VisualizationPage.cpp, MediaPageConstants.h, ThemeIcons.cpp et ThemeConstants.h pour charger Assets à côté de l’exécutable.
  - [x] Modifier src/App/CMakeLists.txt pour compiler les contrôles partagés dans App, retirer les assets du thème et les livrer dans Assets à la compilation et à l’installation.

- [x] Gestion de la session indépendante du thème
  - [x] Créer src/App/Shared/Emulation/EmulationWorkspace.h et .cpp et les ajouter à src/App/CMakeLists.txt pour la découverte des modules, les chemins de profils vidéo et l’application des réglages.
  - [x] Modifier src/App/Themes/RetroMediaLab/Views/EmulationPage.cpp pour appeler ces opérations depuis le dialogue.
  - [x] Déplacer l’écriture des captures PNG et la génération de leur nom de EmulationPage.cpp vers EmulationWorkspace.h/.cpp.

- [x] PNG distribués par catégorie
  - [x] Déplacer src/App/Assets/FileTypes vers src/App/Assets/Icons/FileTypes et corriger le chemin dans MediaPageConstants.h.
  - [x] Copier les PNG absents de la branche C# osd-support sous src/App/Assets/Controllers, src/App/Assets/Media et src/App/Assets/Icons.

Le build Release et son installation locale ont été vérifiés. Les 266 PNG d’App sont externes et chaque copie a été comparée à sa source. Les données du thème RetroMediaLab sont sous Assets/Themes/RetroMediaLab. Les sons mécaniques et les définitions de disques sont livrés par MediaEngine.

- [x] Base des manettes externe
  - [x] Déplacer gamecontrollerdb.txt et SDL_GameControllerDB.LICENSE de src/Platform/Windows/ControllerDatabase vers src/App/Assets/Input.
  - [x] Modifier src/Platform/CMakeLists.txt et src/Platform/Windows/Gamepads.cpp pour lire la base externe installée par App.

Debug et Release livrent les mêmes assets App. La base SDL et sa licence sont externes sous Assets/Input. La vérification du démarrage Release a ouvert la fenêtre principale avec l’accès Windows normal aux préférences.
