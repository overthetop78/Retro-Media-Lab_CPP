# Exécution de la reconnaissance commune

- [x] Résultats et erreurs
  - [x] Créer `src/MediaEngine/src/Images/RecognitionResult.h`, `RecognitionResult.cpp`, `ReadRejection.h` et `ReadRejection.cpp` : résultat possédant reader/document/échecs, exception agrégée structurée, classement explicite des rejets de conteneurs déjà portés ; ne pas absorber E/S, annulation, débordement ou erreurs d’arguments.
  - [x] Modifier les diagnostics Engine et créer les catalogues `src/Translations/Catalogs/Constants/MediaEngine/recognition.json` et `Languages/<langue>/MediaEngine/recognition.json` : erreurs de candidat absent, candidats rejetés et document nul, traduites et relues.
- [x] Exécution et nettoyage
  - [x] Créer `src/MediaEngine/src/Sources/DriveOperation.h`, modifier `src/MediaEngine/src/Images/ReaderRegistry.h` et `ReaderRegistry.cpp` : exécuter les candidats classés, garder les échecs par reader et les scopes d’opération/accès, avec nettoyage normal, exception et initialisation partielle.
- [x] Intégration
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler la reconnaissance et régénérer les projets Visual C++.
- [x] Vérification ciblée
  - [x] Créer `build/verification-recognition/CMakeLists.txt` et `main.cpp` : vérifier repli sur rejet de format, collecte des échecs, absence de repli sur E/S/annulation, et équilibre des scopes du lecteur sur succès et échec.
  - [x] Supprimer `build/verification-recognition/` après consignation du résultat.
