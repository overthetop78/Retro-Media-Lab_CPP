# Primitives de flux et codec GCR partagé

- [x] Modèles et reconstruction
  - [x] Créer `src/MediaEngine/src/Models/Flux/Constants.h`, `Flux.h`, `Bitstream.cpp`, `Timing.cpp`, `Transitions.cpp`, `Revolution.cpp` : bits possédés, intervalles, estimation FM/non-FM/NRZI, reconstruction adaptative/PLL et création de révolution.
  - [x] Créer `src/MediaEngine/src/Decoding/Confidence.h`, `Confidence.cpp`, `Constants.h` : calculs de confiance communs aux décodeurs.
- [x] GCR partagé
  - [x] Créer `src/MediaEngine/src/Codecs/Commodore/Gcr.h`, `Constants.h`, `Gcr.cpp` : table 4/5 bits, encodage, décodage avec pas, écriture positionnée et checksum XOR.
- [x] Construction et messages
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` et régénérer les projets Visual C++ : compilation des primitives et diagnostics codés.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/flux-core.json` et les catalogues des 29 langues : erreurs de durées, limites et coordonnées, génération Argos et relecture.
- [x] Validation
  - [x] Créer `build/verification-flux-core/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp` et `CMakeLists.txt` : compiler les sources C# originales, comparer timings, bits, intervalles, GCR, checksums, confiance et cas limites en Debug et Release.
  - [x] Supprimer `build/verification-flux-core/` et les journaux temporaires `build/flux-core-reference.log`, `build/flux-core-bench-debug.log`, `build/flux-core-bench-result.log`, `build/flux-core-bench-release.log` après validation.
