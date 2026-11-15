# Conteneurs Atari ATR et ATX

- [x] Géométrie et lecture/écriture ATR
  - [x] Créer `src/MediaEngine/src/Formats/Atari8Bit/Geometry.h`, `Geometry.cpp`, `Constants.h` : partager géométries Atari, tailles d’amorçage, profils et identifiants de capacité déjà centralisés.
  - [x] Créer `src/MediaEngine/src/Formats/Atr/Atr.h`, `Constants.h`, `Layout.cpp`, `Reader.cpp` : valider le conteneur, récupérer les variantes tronquées/paddées reconnues par le C#, reconstruire les secteurs physiques/logiques et la capacité exacte.
  - [x] Créer `src/MediaEngine/src/Formats/Atr/Writer.cpp` : valider le profil catalogué et chaque secteur, écrire les paragraphes et tailles d’amorçage par le service atomique existant.
- [x] Lecture ATX et protection
  - [x] Créer `src/MediaEngine/src/Formats/Atx/Atx.h`, `Constants.h`, `Reader.cpp` : valider les pistes et offsets, respecter le statut missing-data 0x10, sélectionner les doublons selon statut et position en ordre stable, conserver l’état d’intégrité et le diagnostic du candidat retenu.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, diagnostics et allowlist ; créer les catalogues de traduction ATR/ATX : intégration par l’agent principal.
  - [x] Modifier `src/Translations/Catalogs/Languages/*/MediaEngine/atr-atx.json` : relire/corriger les traductions Argos et vérifier clés/paramètres/invariants.
- [x] Vérification ciblée
  - [x] Créer `build/verification-atr-atx/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, cas et manifestes : comparer les vraies sources C# ATR/ATX, récupération des variantes ATR, protection/ordre/sélection ATX, limites et annulation, écritures atomiques ATR.
  - [x] Supprimer `build/verification-atr-atx/` et les artefacts temporaires après validation.
