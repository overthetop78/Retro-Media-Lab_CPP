# Registre d’encodage des pistes

- [x] Sélection et validation
  - [x] Créer `src/MediaEngine/src/Encoding/Registry.h`, `Registry.cpp` : catalogue par défaut ou collection explicite, copie ordonnée, validations séquentielles C#, recherche exacte et encodage avec durée de vie maîtrisée.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp` : compiler le registre et déclarer les quatre diagnostics spécifiques aux encodeurs.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/flux-encoder-registry.json` et ses 29 catalogues : entrée nulle, identifiant vide, doublon et identifiant absent ; Argos puis relecture.
- [x] Vérification
  - [x] Créer `src/MediaEngine/src/Primitives/Identifiers.h` et modifier `Encoding/Registry.cpp`, `Decoding/Registry.cpp` : préserver U+FEFF initial pendant la validation UTF-8 des identifiants, avec une fonction commune aux deux registres et des cas de comparaison C#.
  - [x] Créer `build/verification-encoder-registry/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer les classes C# originales avec collections explicites, erreurs et priorités, ordre, casse/Unicode, identifiants modifiés après construction et délégation de l’encodage ; vérifier catalogue complet, propriété et annulation Debug/Release.
  - [x] Supprimer `build/verification-encoder-registry/` et ses journaux après validation : retrait du banc et des artefacts temporaires.
