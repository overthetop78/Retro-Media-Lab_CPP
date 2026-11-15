# TeleDisk : enregistrements et compression sectorielle

- [x] Enregistrements TeleDisk
  - [x] Créer `src/MediaEngine/src/Formats/TeleDisk/Constants.h` et `Records.h` : conserver les champs d’en-tête, commentaire binaire, piste et secteurs, leurs drapeaux et les constantes du format C#.
- [x] Charges utiles sectorielles
  - [x] Créer `src/MediaEngine/src/Formats/TeleDisk/SectorCodec.h` et `SectorCodec.cpp` : porter CRC TeleDisk, décodage brut/mot répété/RLE et sélection sans perte de l’encodage d’écriture.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h` et `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/teledisk-sector.json` et les catalogues `Languages/<langue>/MediaEngine/teledisk-sector.json` : erreurs d’encodage traduites avec adresses et longueurs.
- [x] Intégration et contrôle
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py` et `scripts/translations/ignores/common.txt` : compiler les nouveaux fichiers et protéger TeleDisk.
  - [x] Créer `build/verification-teledisk-sector/CMakeLists.txt` et `main.cpp` : vérifier une fois les trois encodages, CRC, troncature et longueur finale sur données en mémoire.
  - [x] Supprimer `build/verification-teledisk-sector/` après consignation des résultats.

La compression avancée LZSS/Huffman, le lecteur détaillé et le writer de conteneur suivent ce bloc ; aucun lecteur TeleDisk partiel n’est publié ici.
