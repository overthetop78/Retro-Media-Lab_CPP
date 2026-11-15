# Classement des readers communs

- [x] Présélection
  - [x] Créer `src/MediaEngine/src/Images/ReaderRegistry.h` et `ReaderRegistry.cpp`, modifier `Constants.h` : posséder la liste des readers et calculer les candidats avec les cinq preuves et priorités du C#, puis trier par confiance, nombre de preuves et ordre d’enregistrement. Conserver les raisons sous forme d’énumération indépendante du texte affiché.
  - [x] Modifier les diagnostics Engine et créer `src/Translations/Catalogs/Constants/MediaEngine/reader-selection.json` et `Languages/<langue>/MediaEngine/reader-selection.json` : traduire le rejet d’un reader nul.
- [x] Intégration
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler le classement commun et régénérer les projets Visual C++.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-reader-selection/CMakeLists.txt` et `main.cpp` : vérifier classement, égalités stables, format explicite, fichiers associés, signatures de tailles différentes et propagation des erreurs/annulation.
  - [x] Supprimer `build/verification-reader-selection/` après consignation du résultat.
