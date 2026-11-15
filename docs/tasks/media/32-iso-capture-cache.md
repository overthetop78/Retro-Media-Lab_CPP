# Cache des candidats ISO et entrée fichier

- [x] Service de décodage
  - [x] Créer `src/MediaEngine/src/Reconstruction/Iso/CandidateDecoder.h`, `CaptureCache.cpp` et modifier `Constants.h` : service possédé, clé ordonnée des codecs, empreinte chemin absolu/taille/date, partage concurrent du résultat ou de l’exception, invalidation et retrait conditionnel des échecs ; lecteur SCP injecté ou réel, adaptation puis collecte, surcharge image sans cache.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler le service avec les sources communes, sans logique dans App.
- [x] Validation
  - [x] Créer `build/verification-iso-cache/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt` : scénarios sur le collecteur/cache C# original et doubles contrôlés des dépendances ; hits, changements de fichier/codecs, concurrence, progression et token du premier appel, échec/annulation/réessai, remplacement d’une entrée active ; intégration d’un vrai fichier SCP FM/MFM et annulation/RAII en Debug/Release.
  - [x] Supprimer `build/verification-iso-cache/`, `build/iso-cache-reference.log`, `build/iso-cache-bench-debug.log`, `build/iso-cache-bench-release.log` : retrait du banc, captures synthétiques et artefacts.

Les appels du service C++ sont synchrones, comme les lecteurs/collecteurs déjà portés ; les demandeurs simultanés partagent le même travail via un résultat différé. Aucun thread détaché ou tâche conservant des références vers un service détruit n’est créé. Le registre général des codecs sera porté séparément.
