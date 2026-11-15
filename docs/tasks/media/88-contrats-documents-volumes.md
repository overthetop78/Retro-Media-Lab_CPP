# Contrats neutres de documents et de volumes

- [ ] Contrats publics fidèles au C#
  - [ ] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Representations.h` : interfaces en lecture seule pour représentation générique, blocs, secteurs, pistes optiques et représentation optique ; lectures asynchrones par futures, annulation explicite et données possédées pour le retour des pistes.
  - [ ] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/MediaDocument.h` : interfaces de source et document reconnus, chemins/longueurs, format/type, représentation, volumes, diagnostics et métadonnées en lecture seule sans dépendance à Engine.
  - [ ] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Volumes.h` et `src/MediaFileSystems/src/Partitioning/Volumes.cpp` : remplacer le descripteur mutable partiel par une valeur construite/validée, API en lecture seule et copie/déplacement compatibles avec le tri ; valider origine Unicode, coordonnées et numéros optionnels.
  - [ ] Modifier `src/MediaFileSystems/src/Partitioning/Mbr.cpp`, `Gpt.cpp`, `Helpers.h` : construire les mêmes descripteurs par le contrat validé et adapter leurs accesseurs, préserver décisions, ordre et diagnostics ; rejeter les coordonnées hors domaine signé C#.
  - [ ] Modifier les diagnostics MFS partagés (agent87) et créer `src/Translations/Catalogs/Constants/MediaFileSystems/volume-contract.json`, puis les catalogues de langues : erreurs structurées traduisibles et catégories précises.
  - [ ] Modifier les filtres de corruption de `src/MediaFileSystems/src/Partitioning/Mbr.cpp` et `Gpt.cpp` : capturer exactement `ErrorKind::InvalidData` comme les catch C# originaux, propager E/S, arguments et annulation ; ajouter leurs cas ciblés au banc88.
- [ ] Vérification ciblée
  - [ ] Créer `build/verification-volume-contract/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, cas/manifestes : comparer le vrai constructeur C# sur limites et Unicode, vérifier les interfaces avec représentations dérivées et lectures asynchrones, et vérifier des sorties MBR/GPT/volume entier après changement du contrat.
  - [x] Supprimer `build/verification-volume-contract/` et ses artefacts après validation et intégration des empreintes.
