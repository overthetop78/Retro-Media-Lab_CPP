# Alcohol MDS/MDF

- [x] Descripteur binaire
  - [x] Créer `src/MediaEngine/src/Formats/Optical/Alcohol/Constants.h` : signature, offsets, dimensions, modes et métadonnées du C#.
  - [x] Créer `src/MediaEngine/src/Formats/Optical/Alcohol/Descriptor.h`, `Descriptor.cpp` : en-tête, sessions possédées, blocs de pistes, lead-out, footers et noms Latin-1/UTF-16LE bornés ; profil autonome CD du C#.
- [x] Raccordement optique
  - [x] Créer `src/MediaEngine/src/Formats/Optical/Alcohol/Reader.h`, `Reader.cpp` : reconnaissance, fichiers MDF partagés, fenêtres utilisateur, sous-canaux entrelacés, limites de sessions/pistes et métadonnées.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, `src/MediaEngine/src/Images/ReadRejection.cpp` et créer `src/Translations/Catalogs/Constants/MediaEngine/alcohol-mds.json`, `src/Translations/Catalogs/Languages/*/MediaEngine/alcohol-mds.json` : erreurs traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `scripts/translations/ignores/common.txt`, `RetroMediaLab.vcxproj` et `.filters` : compilation, constantes, invariants MDS/MDF et navigation Visual C++.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-alcohol-mds/CMakeLists.txt`, `main.cpp` : structures/footers, chemins et profils, cinq modes de piste et sous-canaux entrelacés, sources partagées, bornes et annulation ; fixtures temporaires.
  - [x] Supprimer `build/verification-alcohol-mds/` avec ses projets, binaires, MDS/MDF temporaires après consignation.
