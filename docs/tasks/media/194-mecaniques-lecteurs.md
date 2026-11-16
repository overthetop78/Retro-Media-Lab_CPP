# Mécaniques et composition des lecteurs

- [x] État et audio des lecteurs
  - [x] Créer `src/MediaEngine/src/Drives/Mechanics.h`, `Mechanics.cpp` et `Audio.cpp` : implémenter DriveMechanics suivant les deux fichiers MediaDriveMechanics C#, état verrouillé, opérations et accès imbriqués, événements, voix de déplacement, bandes et mixage limité sans sortie audio possédée.
  - [x] Modifier `src/MediaEngine/src/Audio/Mechanical/Constants.h` : opérations, volume initial et nombre maximal de voix nommés.
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/DriveService.h` et `src/MediaEngine/src/Drives/Service.cpp` : créer les lecteurs de disquettes, HDD et optiques, relayer les notifications sans conserver les lecteurs.
  - [x] Modifier `src/MediaEngine/src/Composition/Recognition.h` et `Recognition.cpp` : utiliser la fabrique concrète du service dans la lecture par défaut.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` et `src/Translations/Catalogs/Constants/MediaEngine/mechanical-audio.json` puis `Languages/<langue>/MediaEngine/mechanical-audio.json` : erreur traduite de paramétrage mécanique.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compilation des composants et catalogues.
  - [x] Modifier `scripts/translations/translate.py` : charger les ressources Stanza déjà installées sans téléchargement implicite lors de la traduction Argos.
- [x] Vérification ciblée
  - [x] Créer `build/verification-drive-mechanics/CMakeLists.txt` et `main.cpp` : état, accès imbriqués, notification hors verrou, moteur et transitoires, déplacements concurrents, bandes, désactivation et remise à zéro, fabrique et rattachement à la lecture par défaut.
  - [x] Supprimer `build/verification-drive-mechanics/` après utilisation avec les exécutables temporaires.
