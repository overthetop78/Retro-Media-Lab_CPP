# Exploration du contenu séquentiel dans MFS

- [x] Blocs décodés et fichiers nommés
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/SequentialContent.h` et `src/MediaFileSystems/src/Exploration/Sequential/Block.cpp`, `Constants.h`, `Reader.cpp` : blocs possédés immuables, contrat du contenu décodé et regroupement fidèle des fichiers nommés.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaFileSystems/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaFileSystems/sequential-content.json` et `src/Translations/Catalogs/Languages/*/MediaFileSystems/sequential-content.json` : validation et avertissements traduisibles.
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj`, `.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-sequential-content/CMakeLists.txt`, `main.cpp` : noms/numéros/inconnus, ordre, intégrité à trois états, métadonnées, propriétés du volume et détecteur séquentiel existant.
  - [x] Supprimer `build/verification-sequential-content/` et ses artefacts temporaires après consignation.

Validation et limites dans docs/validation/media/sequential-content.md.
