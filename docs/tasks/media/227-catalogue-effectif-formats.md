# Catalogue effectif des formats

- [x] État cohérent du catalogue
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/ImageFormatWorkspace.h`, modifier `ImageFormatDetection.h` : instantané possédé des capacités, catalogue et détecteur ; accès au catalogue déjà possédé par le détecteur sans copie supplémentaire.
  - [x] Créer `src/MediaEngine/src/Formats/Catalog/Workspace.cpp` et modifier `Constants.h` : fusion des définitions intégrées, capacités fournies et formats physiques, ajout de définitions utilisateur, extensions de repli et reconstruction atomique de l'instantané.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `.filters` : intégrer le service.
- [x] Vérification ciblée
  - [x] Créer `build/verification-format-workspace/CMakeLists.txt` et `main.cpp` : catalogue initial, capacités réelles séparées des capacités effectives, cohérence détecteur/catalogue, remplacement et reconstruction, définition importée et conservation de l'ancien instantané sur erreur.
  - [x] Supprimer `build/verification-format-workspace/` et ses artefacts temporaires après utilisation.
