# Registre de décodage séquentiel et adaptateur MFS

- [x] Contrats et sélection des décodeurs
  - [x] Créer `src/MediaEngine/src/Decoding/Sequential/Models.h`, `Models.cpp`, `Decoder.h`, `Registry.h`, `Registry.cpp`, `Constants.h` : blocs/résultats possédés, positions sources, confiance, contrats et sélection stricte par machine.
  - [x] Créer `src/MediaEngine/src/Images/SequentialContentAdapter.h`, `.cpp` : compatibilité du volume, exécution des candidats, classement stable, conversion vers le contrat MFS et diagnostics traduisibles.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/sequential-decoding.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/sequential-decoding.json` : validation, rejets et sélection traduits.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj`, `.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-sequential-decoding/CMakeLists.txt`, `main.cpp` : contrats, sélection 0/1/plusieurs, classement avec blocs/confiance/taille, égalités, diagnostics et passage à l’exploration MFS existante.
  - [x] Supprimer `build/verification-sequential-decoding/` et ses artefacts temporaires après consignation.

Validation et limites dans docs/validation/media/sequential-decoding.md.
