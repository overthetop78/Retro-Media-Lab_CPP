# Lecteur SCP commun et cache partagé

- [x] Identité des fichiers et cache
  - [x] Créer `src/MediaEngine/src/Sources/FileIdentity.h` et `FileIdentity.cpp`, modifier `Reconstruction/Iso/CaptureCache.cpp` : partager la lecture de l'identité chemin/taille/date, sans dupliquer son implémentation entre caches.
  - [x] Créer `src/MediaEngine/src/Formats/Scp/FileCache.h` et `FileCache.cpp` : chargement unique partagé par version, mémorisation directe, invalidation des anciennes versions, retrait des échecs, annulation d'une attente indépendante et libération RAII des travailleurs.
- [x] Représentation et lecteur
  - [x] Créer `src/MediaEngine/src/Formats/Scp/CaptureReader.h`, modifier `Scp.h`, `FileCache.h` et `Adapter.h` : porter le contrat injectable de lecture de capture et sa méthode Remember optionnelle, sans dépendre de l'implémentation du cache.
  - [x] Modifier `src/MediaEngine/src/Models/Flux/ProtectedTrack.h` et `ProtectedTrack.cpp` : permettre le partage immuable d'une révolution avec son propriétaire, conserver le constructeur par valeur et rejeter les pointeurs nuls.
  - [x] Modifier `src/MediaEngine/src/Formats/Scp/ProtectedImage.h` et `ProtectedImage.cpp` : exposer une conversion partageant les révolutions d'une capture SCP possédée.
  - [x] Modifier `src/MediaEngine/src/Formats/Scp/Adapter.h` et `Adapter.cpp`, créer `Metadata.cpp`, modifier `Constants.h` : raccorder reconnaissance, lecture, cache et métadonnées au contrat ImageReader ; exposer aussi la lecture de fichier et Remember.
  - [x] Modifier les diagnostics Engine (`Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`) et créer les catalogues `Constants/MediaEngine/scp-cache.json`, `flux-revolution-ownership.json` et leurs équivalents `Languages/<langue>/MediaEngine/` : erreurs d'argument du cache et de révolution nulle traduites.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler identité, cache et métadonnées.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-scp-reader-cache/CMakeLists.txt` et `main.cpp` : vérifier ouverture via le registre, métadonnées, partage de durée de vie, réutilisation/invalidation/reprise du cache et attente annulée sans arrêter l'autre appelant.
  - [x] Supprimer `build/verification-scp-reader-cache/` et ses fichiers/binaires temporaires après consignation.
