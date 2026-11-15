# Représentation sectorielle et liaison mécanique

- [x] Adresses sans lecture de charge utile
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/SectorImage.h`, `src/MediaEngine/src/Models/Sectors/SectorImages.cpp`, `src/MediaEngine/src/Formats/CommodoreDisks/Reader.cpp` et `src/MediaFileSystems/src/FileSystems/Acorn/Adfs/BlockView.cpp` : exposer l’adresse d’un bloc disponible sans lire ses octets ; déléguer cette adresse dans les vues renommées et conserver les adresses zonées/explicites.
- [x] Document et accès mécaniques
  - [x] Créer `src/MediaEngine/src/Models/Sectors/Representation.h` et `Representation.cpp` : porter la vue sectorielle Engine/MFS, posséder l’image sans copier ses blocs, lier une mécanique avec une granularité de lecture sectorielle et terminer les accès sur sortie normale/exception ; conserver la géométrie et les métadonnées des blocs.
  - [x] Modifier `src/MediaEngine/src/Images/MediaDocument.cpp` et `src/MediaEngine/CMakeLists.txt` : intégrer la représentation et terminer la sélection des liaisons blocs/secteurs/optique.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-sector-representation/CMakeLists.txt` et `main.cpp` : vérifier les adresses sans E/S, les lectures physiques/logiques, l’identité des blocs partagés, l’idempotence et le nettoyage sur exception ; raccorder une image à la détection générique des volumes sans rejouer les anciennes suites.
  - [x] Supprimer `build/verification-sector-representation/` : retirer le banc et ses artefacts après consignation des résultats.
