# Frontière d'exploration et conversion des volumes

- [x] Diagnostics et données échangées
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h` et `src/Diagnostics.cpp` : conserver une recette de traduction externe avec ses arguments et son domaine, sans dépendance vers MediaEngine.
  - [x] Modifier `src/MediaEngine/src/Images/ExplorationMapper.h` et `ExplorationMapper.cpp` : conversion inverse complète des volumes publics, entrées, dates, messages et diagnostics imbriqués ; restituer les recettes externes.
  - [x] Modifier `src/MediaEngine/src/Images/Creation/Migration.cpp` : réutiliser le convertisseur commun au lieu d'une projection privée des entrées.
- [x] Composition et erreurs de lecture des bandes
  - [x] Modifier `src/MediaEngine/src/Images/SequentialContentAdapter.cpp`, `Images/Constants.h` et `MediaExplorer.cpp` : enregistrer les décodeurs par défaut et convertir les erreurs de données invalides identifiées dans le C# en diagnostics MFS, sans absorber les annulations, erreurs d'accès ou de programmation.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-exploration-boundary/CMakeLists.txt` et `main.cpp` : aller-retour des volumes et recettes imbriquées, propriété des contenus, erreur de décodage rapportée, propagation des autres erreurs et exploration séquentielle par défaut.
  - [x] Supprimer `build/verification-exploration-boundary/` et ses sorties après utilisation.
