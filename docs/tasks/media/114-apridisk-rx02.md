# Apridisk et dump physique RX02

- [x] Apridisk
  - [x] Créer `src/MediaEngine/src/Formats/Apridisk/Constants.h`, `Apridisk.h`, `Internal.h`, `Reader.cpp` et `Writer.cpp` : en-tête ACT, enregistrements secteurs bruts/compressés, doublons remplacés comme en C#, validation de la géométrie complète et écriture atomique native.
- [x] RX02
  - [x] Créer `src/MediaEngine/src/Formats/Rx02/Constants.h`, `Rx02.h`, `SectorOrder.h`, `Reader.cpp` et `Writer.cpp` : reprendre l’entrelacement/skew et rotation de piste, construire les blocs RT-11 et écrire le dump physique inverse.
- [x] Contrats et diagnostics
  - [x] Créer `src/MediaEngine/src/Formats/Apridisk/Adapter.h`, `Adapter.cpp`, `src/MediaEngine/src/Formats/Rx02/Adapter.h` et `Adapter.cpp` : raccorder capacités, sondes et documents aux contrats communs.
  - [x] Modifier les diagnostics Engine et `src/MediaEngine/src/Images/ReadRejection.cpp`, créer les catalogues `src/Translations/Catalogs/Constants/MediaEngine/apridisk-rx02.json` et `Languages/<langue>/MediaEngine/apridisk-rx02.json` : traduire les erreurs des deux conteneurs.
  - [x] Modifier `scripts/translations/ignores/common.txt` : protéger les noms techniques ApriDisk, ACT Apricot et ACT Apricot PC/Xi pour les nouvelles traductions.
- [x] Intégration
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` et `scripts/quality/audit_literals.py` : compiler les lecteurs/writers, déclarer leurs constantes et régénérer les projets Visual C++.
  - [x] Modifier `src/MediaEngine/src/Formats/Raw/ImgReader.cpp` : renommer la variable statique des capacités pour corriger l’avertissement de masquage détecté lors de la recompilation.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-apridisk-rx02/CMakeLists.txt` et `main.cpp` : vérifier décompression/enregistrements Apridisk et adresses RX02 avec lecture/réécriture native et sondes communes.
  - [x] Supprimer `build/verification-apridisk-rx02/` et ses sorties synthétiques après consignation.
