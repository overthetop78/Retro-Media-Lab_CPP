# Définitions intégrées et arguments externes

- [x] Porter le contrat de préparation des définitions intégrées
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/BuiltInDiskDefinitions.h` : option activée avec valeur facultative, sélection des quatre formats et ajout conditionnel des arguments.
  - [x] Créer `src/MediaEngine/src/Formats/Definitions/BuiltInConstants.h` et `BuiltInArguments.cpp` : chemin relatif portable, comparaison insensible à la casse, respect d'une option explicite et ajout de `--diskdefs`.
  - [x] Créer `src/MediaEngine/Assets/DiskDefinitions/built-in.cfg` : reprendre exactement les quatre définitions C#.
  - [x] Modifier `src/MediaEngine/src/Formats/Catalog/Greaseweazle.cpp` et `src/MediaEngine/src/Visualization/Classification.cpp` : utiliser la même sélection publique sans règle parallèle.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : intégrer la source, les constantes et la distribution du CFG au build et à l'installation.
- [x] Vérifier les arguments et la distribution
  - [x] Créer `build/verification-built-in-arguments/CMakeLists.txt` et `main.cpp` : quatre formats, casse, format inconnu, option explicite et lecture du CFG livré dont le chemin est reçu en argument.
  - [x] Supprimer `build/verification-built-in-arguments/` après utilisation.
