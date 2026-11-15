# Registre de décodeurs de flux et sélection

- [x] Contrats et sélection
  - [x] Créer `src/MediaEngine/src/Decoding/Decoder.h`, `Registry.h`, `Scoring.h`, `Scoring.cpp`, `ScoringConstants.h` : interface de décodeur, résultats partagés immuables, scores automatique/explicite C#, comparaisons stables et NaN, exclusion déclarée des codecs réservés au choix explicite.
  - [x] Modifier `src/MediaEngine/src/Models/Flux/Flux.h`, `Reconstruction/Iso/Collector.h`, `Collector.cpp` : référence possédée commune à une fenêtre pour tous ses décodeurs, contrat de service compatible avec le cache par identité sans emprunt pendant.
- [x] Cache et registre
  - [x] Créer `src/MediaEngine/src/Decoding/Registry.cpp` : validation complète avant indexation ordinale, registre ordonné, cache faible par objet révolution/codec, partage concurrent des succès et exceptions, sélection automatique et meilleure révolution ; callback possédant son service pour la collecte ISO.
- [x] Construction et diagnostics
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py` : compilation, constantes et diagnostics structurés.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/flux-registry.json` et les 29 catalogues : traduction Argos puis relecture des validations et sélections impossibles.
- [x] Validation
  - [x] Créer `build/verification-flux-registry/Reference.csproj`, `Reference.cs`, `generate.py`, `main.cpp`, `CMakeLists.txt` : registre/scoring C# originaux avec codecs de contrôle, scores et tie-breaks, identité/concurrence/exceptions/durée de vie, contrats Unicode, sélections vides et intégration à la collecte/cache ISO avec les codecs réels ; DLL de langues Debug/Release.
  - [x] Supprimer `build/verification-flux-registry/`, `build/flux-registry-reference.log`, `build/flux-registry-bench-debug.log`, `build/flux-registry-bench-release.log` : retrait du banc et des artefacts.

Le catalogue par défaut ne sera pas remplacé par un sous-ensemble silencieux : ses 25 codecs seront assemblés une fois portés. Le registre explicite reprend les opérations source ; le constructeur sans argument restera non porté dans l’inventaire. L’exclusion automatique de Lisa sera portée via le contrat de participation de son codec, plutôt qu’une dépendance du registre à son type concret.
