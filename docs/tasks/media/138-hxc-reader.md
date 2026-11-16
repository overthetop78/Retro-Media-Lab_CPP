# Lecture HxC Stream

- [x] Décodage et révolutions
  - [x] Modifier `src/MediaEngine/src/Formats/HxcStream/Constants.h`, `Stream.h`, `Internal.h`, créer `Reading.cpp`, `Pulses.cpp`, `Revolutions.cpp` et `Metadata.cpp` : chunks CRC, blocs LZ4 bornés, impulsions variables, métadonnées et reconstruction des révolutions entre index ou depuis les impulsions.
- [x] Jeu de pistes et registre
  - [x] Créer `src/MediaEngine/src/Formats/HxcStream/Files.cpp`, `Adapter.h` et `Adapter.cpp` : découverte du même préfixe physique, classement des pistes, capture native base64 et empreinte, protection globale, source associée et lecture par le registre.
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`), `src/MediaEngine/src/Images/ReadRejection.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/hxc-reading.json` et ses équivalents `Languages/<langue>/MediaEngine/` : erreurs du lecteur traduites.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` et les projets/filtres Visual C++ générés : compiler les sources du lecteur.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger le suffixe technique NN.S.hxcstream contre les altérations des modèles de traduction.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-hxc-reader/CMakeLists.txt` et `main.cpp` : lecture de plusieurs pistes, métadonnées, révolutions et protection, CRC/blocs/impulsions invalides, annulation et registre.
  - [x] Supprimer `build/verification-hxc-reader/` et ses fichiers/binaires temporaires après consignation.

L'écriture du jeu multipiste et sa publication atomique restent distinctes de ce bloc.
