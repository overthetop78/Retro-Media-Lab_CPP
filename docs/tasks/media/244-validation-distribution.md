# Validation de la distribution des bibliothèques médias

- [x] Vérifier le chargement des trois DLL livrées
  - [x] Créer `build/verification-media-release/main.cpp` et compiler `verify.exe` : prendre le dossier distribué en argument, ajouter temporairement les dossiers de dépendances, charger les trois DLL puis les décharger en ordre inverse avec RAII, y compris en cas d'échec ; ne pas ouvrir de fenêtre ni relancer les tests de formats.
  - [x] Supprimer `build/verification-media-release/` et ses artefacts de compilation après utilisation.
