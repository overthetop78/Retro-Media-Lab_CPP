# Encodage séquentiel et Atari CAS



- [x] Contrats et registre d’encodage

  - [x] Créer `src/MediaEngine/src/Encoding/Sequential/Request.h`, `Request.cpp`, `Encoder.h`, `Registry.h`, `Registry.cpp`, `Constants.h`, `Compatibility.h`, `.cpp` : requête possédée, sélection stricte et normalisation CSW/RAW vers WAV du C#.

- [x] Encodage Atari CAS

  - [x] Créer `src/MediaEngine/src/Encoding/Sequential/Atari/Encoder.h`, `Encoder.cpp`, `Records.cpp` : chunks conservés, marque FUJI, ordre stable, durées et reconstruction des records des blocs logiques CAS ; les octets série déjà encadrés restent directs.

  - [x] Modifier `src/MediaEngine/src/Formats/Tape/AtariCas/Constants.h` : paramètres et valeurs d’encodage, métadonnée explicite de records reconstruits.

  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp` ; créer `src/Translations/Catalogs/Constants/MediaEngine/sequential-encoding.json` et `src/Translations/Catalogs/Languages/*/MediaEngine/sequential-encoding.json` : validation et rejets traduisibles.

  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj`, `.filters` : compilation et constantes.

- [x] Contrôle ciblé

  - [x] Créer `build/verification-sequential-encoding/CMakeLists.txt`, `main.cpp` : propriété, registre 0/1/plusieurs, normalisation, chunks retenus/ordre/durées, aller-retour records CAS et annulation.

  - [x] Supprimer `build/verification-sequential-encoding/` et ses artefacts temporaires après consignation.



- [x] Éviter les recopies de données pendant la sélection
  - [x] Modifier `src/MediaEngine/src/Decoding/Sequential/Models.h`, `Models.cpp` : partager les octets possédés immuables des blocs lors des copies de requêtes, sans partager de tampon mutable de l’appelant.
