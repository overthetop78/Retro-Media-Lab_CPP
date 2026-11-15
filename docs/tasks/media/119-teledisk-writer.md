# TeleDisk : écriture détaillée et adapter UCSD

- [x] Writer natif
  - [x] Créer `src/MediaEngine/src/Formats/TeleDisk/Writer.h`, `Writer.cpp` et `HeaderFactory.cpp` ; modifier `Constants.h` et `Internal.h` : produire les en-têtes/commentaires/pistes/secteurs, préserver les données détaillées, créer les enregistrements depuis une image sectorielle et écrire atomiquement. Refuser le compte de 255 secteurs par piste, réservé au marqueur terminal 0xFF.
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`) et créer les catalogues `Constants/MediaEngine/teledisk-writer.json` et `Languages/<langue>/MediaEngine/teledisk-writer.json` sous `src/Translations/Catalogs/` : erreurs d’écriture traduites et conversions d’octets contrôlées.
- [x] Contrat commun
  - [x] Créer `src/MediaEngine/src/Formats/Raw/UcsdWriter.h` et `UcsdWriter.cpp` ; modifier `Raw/Constants.h` : choix IMG linéaire ou TD0, filtre cible UCSD et validation des charges utiles comme dans la composition C#.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler le writer, la fabrique d’en-tête et l’adapter UCSD.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-teledisk-writer/CMakeLists.txt` et `main.cpp` : vérifier une réécriture détaillée, la fabrique d’en-tête, les secteurs enregistrés et manquants, les refus de données incohérentes et le filtre du writer UCSD.
  - [x] Supprimer `build/verification-teledisk-writer/` après consignation.
