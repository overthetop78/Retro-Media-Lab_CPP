# Conteneur Atari ST MSA

- [x] Lecture et écriture structurées
  - [x] Créer `src/MediaEngine/src/Formats/St/Geometry.h`, `Geometry.cpp`, `Constants.h` : porter la géométrie Atari ST réutilisable, profils catalogués, construction validée et identifiant depuis la capacité, utilisés par MSA.
  - [x] Créer `src/MediaEngine/src/Formats/Msa/Constants.h`, `Msa.h`, `Rle.cpp` : constantes format/layout et compression/décompression RLE fidèle partagée entre reader et writer.
  - [x] Créer `src/MediaEngine/src/Formats/Msa/Reader.cpp` : valider en-tête/géométrie et longueurs, lire les pistes brutes/compressées et construire l’image sectorielle avec adresses/logiques originales.
  - [x] Créer `src/MediaEngine/src/Formats/Msa/Writer.cpp` : valider la géométrie Atari ST existante, réunir les secteurs et choisir la forme la plus courte, écrire par le service atomique existant.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, diagnostics et allowlist ; créer `src/Translations/Catalogs/Constants/MediaEngine/msa.json` et catalogues de langues : intégrer les composants et erreurs traduites (agent principal).
  - [x] Modifier `src/Translations/Catalogs/Languages/*/MediaEngine/msa.json` : relire/corriger les traductions Argos et contrôler paramètres/invariants.
  - [x] Modifier `src/Translations/Catalogs/Languages/*/MediaEngine/st-geometry.json` : relire/corriger les 29 catalogues de géométrie ST, en parallèle par l’agent chargé des systèmes de fichiers.
- [x] Vérification ciblée
  - [x] Créer `build/verification-msa/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, cas et manifestes : comparer les vraies sources C# pour RLE/reader/writer, limites, secteurs absents/courts, annulation et conservation de destination.
  - [x] Supprimer `build/verification-msa/` et les artefacts temporaires après validation.
