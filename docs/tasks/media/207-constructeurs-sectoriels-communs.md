# Constructeurs sectoriels communs

- [x] Centraliser les constructions linÃ©aires, rÃ©guliÃ¨res et Apple
  - [x] CrÃ©er `src/MediaEngine/src/Models/Sectors/Builders.h`, `Builders.cpp` : validation des dimensions/capacitÃ©s, numÃ©rotation 0/1, donnÃ©es possÃ©dÃ©es, octets finaux autorisÃ©s, gÃ©omÃ©trie Macintosh zonÃ©e et projection dense avec positions absentes.
  - [x] Modifier `src/MediaEngine/src/Formats/Raw/Reader.cpp`, `Formats/Apple/RawReader.cpp` : utiliser les constructeurs communs sans charger en mÃ©moire les sources IBM/MSX dÃ©jÃ  accessibles alÃ©atoirement.
  - [x] Modifier `src/MediaEngine/src/Formats/Apple/DecodedTracks.h`, `DecodedTracks.cpp` : sÃ©lection rÃ©utilisable avec prÃ©dicat et densification commune pour les sondes DOS/ProDOS, en conservant la prioritÃ© d'intÃ©gritÃ© du C#.
  - [x] Modifier `Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp` et crÃ©er `Catalogs/Constants/MediaEngine/sector-builders.json`, `Catalogs/Languages/<langue>/MediaEngine/sector-builders.json` : erreur traduite de longueur exacte.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj`, `RetroMediaLab.vcxproj.filters` : intÃ©grer constructeurs et catalogues.
- [x] VÃ©rification ciblÃ©e
  - [x] CrÃ©er `build/verification-sector-builders/CMakeLists.txt`, `main.cpp` : tailles/numÃ©rotation/possession, gÃ©omÃ©trie zonÃ©e, octets finaux, densification partielle/dupliquÃ©e, sÃ©lection Apple, intÃ©gration lecteurs et erreurs ; compiler sources et catalogues.
  - [x] Supprimer `build/verification-sector-builders/` et ses journaux temporaires aprÃ¨s vÃ©rification.
