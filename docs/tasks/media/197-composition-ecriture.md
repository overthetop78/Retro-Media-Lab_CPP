# Composition et service d’écriture

- [x] Écrivains du moteur
  - [x] Créer `src/MediaEngine/src/Composition/Writing.h` et `Writing.cpp` : assembler tous les écrivains natifs dans l’ordre du catalogue C# et exposer registre et service central sans algorithme de format dans la composition.
  - [x] Créer `src/MediaEngine/src/Images/WritingService.h` et `WritingService.cpp` : sélectionner l’écrivain, posséder les dépendances de la tâche, borner les activités mécaniques d’entrée/sortie et vérifier les fichiers produits ; conserver les erreurs des écrivains.
  - [x] Modifier `src/MediaEngine/src/Formats/Cartridge/Readers.h` et `ConsoleConstants.h`, créer `ConsoleWriter.cpp` : écrire les banques console selon storedLength et les couples format/extension du C#, sans ajouter de remplissage.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/image-writing.json` et `Languages/<langue>/MediaEngine/image-writing.json` : diagnostics d’écriture traduits.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compiler la composition, le service, l’écrivain et les catalogues.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/DriveService.h` et `src/MediaEngine/src/Drives/Service.cpp` : transmettre une propriété partagée du lecteur aux notifications publiques, afin que le propriétaire audio puisse conserver la fin des bruitages d’un lecteur de sortie temporaire.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-writing-composition/CMakeLists.txt`, `main.cpp` et de petites images temporaires : registre complet sans doublons, catalogue console, écriture/lecture réelle, demandes refusées, annulation, réponses d’écrivain invalides et libération des activités sur erreur.
  - [x] Supprimer `build/verification-writing-composition/` après utilisation avec ses images et exécutables temporaires.
