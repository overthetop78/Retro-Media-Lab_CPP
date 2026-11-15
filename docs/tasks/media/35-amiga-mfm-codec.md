# Codec Amiga MFM complet

- [x] Primitives et contrats
  - [x] Créer `src/MediaEngine/src/Codecs/Amiga/Mfm.h`, `Constants.h`, `OddEven.cpp`, `Parity.cpp` : séparation/assemblage odd/even, parités des plages, empaquetage piste/face, validations bornées et propriété des données.
- [x] Encodage et décodage
  - [x] Créer `src/MediaEngine/src/Codecs/Amiga/Encoder.cpp` : validations dans l’ordre C#, secteurs de 512 octets, nombre restant, label nul, parités, double synchronisation et gaps via les primitives communes.
  - [x] Créer `src/MediaEngine/src/Codecs/Amiga/Decoder.cpp` : reconstruction adaptative commune, double synchronisation, secteur/en-tête tronqué, intégrité combinée, octets et descriptions localisées, classes Encoder/Decoder concrètes pour le registre.
- [x] Construction et traduction
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compilation, constantes et diagnostics structurés du codec.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/amiga-mfm.json` et les 29 catalogues : quatre erreurs et description de structure, génération Argos puis relecture.
- [x] Validation
  - [x] Créer `build/verification-amiga-mfm/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : sources C# originales sans modification ; matrice odd/even/parités, bornes et ordre des refus, bits/intervalles exacts, corruption/troncature et métadonnées, aller-retour/registre/collecte SCP, annulation et langues Debug/Release.
  - [x] Supprimer `build/verification-amiga-mfm/`, `build/amiga-mfm-reference.log`, `build/amiga-mfm-bench-debug.log`, `build/amiga-mfm-bench-release.log` : retrait du banc et de ses artefacts.

Le catalogue général complet et les politiques de reconstruction Amiga restent distincts de ce codec. Les plages de parité utilisent une validation sans débordement signé ; tout écart de diagnostic sur une entrée C# en débordement sera relevé explicitement dans la validation.
