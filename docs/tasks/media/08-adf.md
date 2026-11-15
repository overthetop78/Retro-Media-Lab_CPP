# Conteneurs ADF — Acorn et géométries Amiga associées

- [x] Schémas et profils partagés
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FileCoreLayout.h` ; modifier `FileCore.h`, `src/FileSystems/Acorn/FileCore/Constants.h`, `Codec.cpp`, `NewMap.cpp` : positions publiques du DiscRecord, somme de contrôle réutilisable et capacité déclarée d'une ancienne carte, sans dupliquer son schéma dans MediaEngine.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FormatIds.h` : identifiants des cinq profils ADFS et des deux profils Amiga.
- [x] Lecture et écriture ADF
  - [x] Créer `src/MediaEngine/src/Formats/Adf/AdfImages.h`, `Constants.h`, `Internal.h`, `Geometry.cpp`, `Partial.cpp`, `Reader.cpp`, `Writer.cpp` : huit profils, détection C#, validation des images partielles par ADFS, lectures paresseuses et écriture sectorielle atomique commune.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` et régénérer les projets Visual C++ : compilation et diagnostics du conteneur.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/adf.json` et ses équivalents dans les 29 langues : tailles, formats et exploration Acorn non applicable, Argos puis relecture.
- [x] Validation
  - [x] Créer `build/verification-adf/` : banc C#/C++ compilant les sources originales ADF, comparant reconnaissance, profils, secteurs/métadonnées, images partielles, exploration Acorn/DOS Plus, octets écrits ; contrôles d'annulation, d'erreurs et de publication atomique.
  - [x] Supprimer `build/verification-adf/` et ses artefacts après validation Debug et Release.
