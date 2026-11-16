# Classification des disques et icônes de médias

- [x] Contrats et catalogue
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/DiskClassification.h` et `src/MediaEngine/src/Formats/Catalog/Classification.cpp` : catalogue possédé, familles distinctes triées selon la culture, alias de formats et protection RWTS18.
  - [x] Créer `src/MediaEngine/src/Visualization/ClassificationConstants.h` et `Classification.cpp` : résolution des décodeurs et catégories physiques depuis machine/format/protection ; décision de conversion SCP suivant les capacités logicielles Greaseweazle existantes.
- [x] Icônes
  - [x] Créer `src/MediaEngine/src/Primitives/Path.h` et modifier `IdentifierConstants.h` : extraction du suffixe et constante de séparateur conformes à `Path.GetExtension` pour les noms commençant ou terminant par un point ; réutiliser ce helper dans les deux décisions de présentation.
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaIconIds.h`, `src/MediaEngine/src/Visualization/IconConstants.h` et `Icons.cpp` : priorité du type de média et du format physique, replis par extension conformes au C#.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : intégrer les sources et constantes.
- [x] Vérification ciblée
  - [x] Créer `build/verification-media-classification/CMakeLists.txt` et `main.cpp` : alias/priorité du catalogue, machines/protections, classification automatique/manuelle, densités, icônes et règles de conversion SCP, sans lecture de média.
  - [x] Supprimer `build/verification-media-classification/` et ses artefacts après utilisation.
