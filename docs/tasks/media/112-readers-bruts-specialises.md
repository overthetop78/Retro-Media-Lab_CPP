# Readers bruts spécialisés et helper de sonde

- [x] Lecteurs spécialisés
  - [x] Créer `src/MediaEngine/src/Formats/Epson/RawReader.h` et `RawReader.cpp`, modifier `Constants.h` : reprendre le choix explicite QX-10, vérifier la longueur, construire les secteurs de tailles variables et conserver leurs adresses.
  - [x] Créer `src/MediaEngine/src/Formats/Raw/SpecializedReaders.h` et `SpecializedReaders.cpp`, modifier `Constants.h` : raccorder COHERENT au backend Commodore 900 et reprendre le profil UCSD IBM MFM de 160 Kio.
- [x] Helper de sonde
  - [x] Créer `src/MediaEngine/src/Images/ReaderValidation.h`, modifier `ReadRejection.h` : reprendre le helper générique C# de validation avec filtrage du format demandé et propagation des erreurs hors rejet de format.
  - [x] Modifier les diagnostics Engine et `ReadRejection.cpp`, créer les catalogues `src/Translations/Catalogs/Constants/MediaEngine/epson-raw.json` et `Languages/<langue>/MediaEngine/epson-raw.json` : traduire les erreurs de format explicite absent et de longueur incompatible.
- [x] Intégration
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler les readers et régénérer les projets Visual C++.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-specialized-raw/CMakeLists.txt` et `main.cpp` : vérifier Epson 320/Logo, UCSD, refus COHERENT invalide et filtrage/erreurs du helper de sonde.
  - [x] Supprimer `build/verification-specialized-raw/` après consignation du résultat.
