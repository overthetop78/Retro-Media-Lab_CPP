# Composition des codecs de flux

- [x] Catalogues et sélection
  - [x] Créer `src/MediaEngine/src/Composition/FluxCodecs.h`, `FluxCodecs.cpp` : construire les 25 décodeurs et 24 encodeurs dans l’ordre exact des catalogues C#, avec instances indépendantes.
  - [x] Modifier `src/MediaEngine/src/Decoding/Registry.h`, `Registry.cpp` : ajouter le constructeur utilisant le catalogue par défaut, sans modifier le registre explicite et son cache.
  - [x] Modifier `src/MediaEngine/src/Codecs/AppleIwm/Gcr.h` : exclure Lisa FileWare de la sélection automatique comme le registre C#, tout en conservant le décodage explicite.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler la composition dans son groupe dédié.
- [x] Vérification
  - [x] Créer `build/verification-flux-composition/expected.json`, `main.cpp`, `CMakeLists.txt` : comparer identifiants, noms et ordre à la composition C# relevée, vérifier instances indépendantes, sélection Lisa explicite/automatique, cache et parcours automatique sur pistes synthétiques.
  - [x] Supprimer `build/verification-flux-composition/` et ses journaux après validation Debug/Release : retrait du banc temporaire et de ses artefacts.
