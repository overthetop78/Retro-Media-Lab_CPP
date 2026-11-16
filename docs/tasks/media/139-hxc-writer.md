# Publication récupérable et writer HxC Stream

- [ ] Jeu de fichiers commun
  - [ ] Créer `src/MediaEngine/src/Sources/AtomicImageSetWriter.h`, `AtomicImageSetWriter.cpp`, `Preallocation.h`, `Platform/Windows/Preallocation.cpp` et `Platform/PreallocationUnavailable.cpp`, modifier `AtomicConstants.h` : validation des sorties, fichiers temporaires, sauvegardes, publication, restauration inverse et nettoyage RAII ; réservation Windows sans changer la longueur logique, adaptation isolée pour les autres systèmes.
- [ ] Writer HxC
  - [ ] Modifier `src/MediaEngine/src/Images/WriterAdapters.h` et `WriterAdapters.cpp` : surcharge flux recevant aussi le document, avec conservation de la surcharge existante.
  - [ ] Modifier `src/MediaEngine/src/Formats/HxcStream/Constants.h` et `Adapter.h`, créer `Writer.cpp` : choix capture native/recodage, refus des régions faibles, noms physiques, publication injectable et registre.
  - [ ] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`), créer `src/Translations/Catalogs/Constants/MediaEngine/atomic-set.json`, `hxc-writing.json` et leurs équivalents `Languages/<langue>/MediaEngine/` : erreurs traduites de validation, allocation, restauration, nettoyage et writer.
  - [ ] Modifier `src/MediaEngine/CMakeLists.txt` et les projets/filtres Visual C++ générés : compiler les nouveaux fichiers et sélectionner l'adaptation de préallocation.
- [ ] Contrôle ciblé
  - [ ] Créer `build/verification-hxc-writer/CMakeLists.txt` et `main.cpp` : conservation native et recodage, publication des pistes, nom et métadonnées invalides, validation du jeu, échec de préparation/publication avec restauration, annulation et préallocation sans allonger le résultat.
  - [ ] Supprimer `build/verification-hxc-writer/` et ses fichiers/binaires temporaires après consignation.
