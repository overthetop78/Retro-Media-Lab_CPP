# Conteneur et encodage MSX CAS

- [x] MSX CAS et encodeur
  - [x] Créer `src/MediaEngine/src/Formats/Tape/MsxCas/Constants.h`, `Reader.h`, `Reader.cpp`, `Writer.cpp` : séparateurs en lecture bornée par blocs, marqueurs binaires/BASIC/ASCII, préfixes conservés et écriture atomique sans timing inventé.
  - [x] Créer `src/MediaEngine/src/Encoding/Sequential/Msx/Encoder.h`, `Encoder.cpp` : groupes CAS validés ou PCM FSK 8N2, porteuses longues/courtes et métadonnées de reconstruction.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/msx-cas.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/msx-cas.json` : erreurs et avertissements traduits.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj`, `.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-msx-cas/CMakeLists.txt`, `main.cpp` : reconnaissance explicite/automatique, frontière de tampon, marqueurs et octets inconnus, séparateurs vides, aller-retour, encodage CAS/PCM FSK et annulation atomique.
  - [x] Supprimer `build/verification-msx-cas/` et ses artefacts temporaires après consignation.
