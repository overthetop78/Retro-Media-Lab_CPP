# Reconstruction SCP Apple

- [x] Reconstruction Apple et routage
  - [x] Créer `src/MediaEngine/src/Reconstruction/Apple/Constants.h`, `Decoder.h`, `Decoder.cpp` : collecte commune, fenêtre continue, progression par piste, sélection stable et métadonnées, essais Macintosh autour de la cellule estimée et score C#.
  - [x] Créer `Reconstruction/Apple/Reconstructors.h`, `AppleII.cpp`, `Macintosh.cpp` : Apple II DOS13/DOS16/ProDOS, RWTS18 et Macintosh/Lisa zoné, capacité, tags, identification et positions absentes.
  - [x] Créer `Reconstruction/Apple/Reader.h`, `Reader.cpp` et modifier `src/MediaEngine/src/Formats/FormatIds.h` : lecture chemin/capture, routage explicite et SOS, sélection automatique par complétude puis blocs disponibles.
  - [x] Modifier `Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp` et `Images/ReadRejection.cpp`, créer `Catalogs/Constants/MediaEngine/apple-scp.json`, `Catalogs/Languages/<langue>/MediaEngine/apple-scp.json` : diagnostics traduits SOS/échec global, causes originales possédées avec identité, classification des rejets SCP sans absorber annulation/accès.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj`, `RetroMediaLab.vcxproj.filters`, `scripts/quality/audit_literals.py` : intégrer lecteurs, constantes et catalogues.
- [x] Vérification ciblée
  - [x] Créer `build/verification-apple-scp/CMakeLists.txt`, `main.cpp` et captures temporaires : collecte/progression, métadonnées, DOS13/DOS16/ProDOS/RWTS18, zones et identification Mac/Lisa, facteurs de cellule/arrêt anticipé, routage SOS, classement automatique et causes conservées ; compiler sources et catalogues.
  - [x] Supprimer `build/verification-apple-scp/` et ses journaux temporaires après vérification.
