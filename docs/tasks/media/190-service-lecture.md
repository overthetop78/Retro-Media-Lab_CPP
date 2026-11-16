# Service global de lecture

- [x] Orchestration
  - [x] Créer `src/MediaEngine/src/Images/ReadingService.h` et `ReadingService.cpp` : lecture asynchrone avec propriété des dépendances, reconnaissance puis liaison au lecteur via DriveFactory, repli RAW audio limité aux refus de lecture sans format imposé.
  - [x] Modifier `src/MediaEngine/src/Composition/Recognition.h` et `Recognition.cpp` : exposer le service de lecture associé au registre existant.
  - [x] Modifier `src/MediaEngine/src/Images/Constants.h` : constante d'extension RAW et identifiant de bande brute.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compiler le service ajouté.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-reading-service/CMakeLists.txt` et `main.cpp` : priorité au disque RAW, repli après refus, format imposé, propagation des erreurs d'accès/configuration et annulation, liaison d'un lecteur fourni ou créé, équilibrage des accès et durée de vie asynchrone.
  - [x] Supprimer `build/verification-reading-service/` et ses sorties après utilisation.
