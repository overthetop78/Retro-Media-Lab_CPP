# Conteneur ImageDisk et géométries Epson

- [x] Modèles et géométries
  - [x] Créer `src/MediaEngine/src/Formats/Epson/Geometry.h`, `Geometry.cpp`, `Constants.h` : porter les profils QX10, pistes/secteurs/offsets et détection exacte réutilisables nécessaires à ImageDisk.
  - [x] Créer `src/MediaEngine/src/Formats/ImageDisk/ImageDisk.h`, `Constants.h`, `Models.cpp` : modèles détaillés possédés, modes, interprétation des neuf types d’enregistrement et reconstruction sectorielle fidèle.
- [ ] Lecture, écriture et métadonnées
  - [x] Créer `src/MediaEngine/src/Formats/ImageDisk/Reader.cpp`, `Writer.cpp` : préserver commentaires ASCII, modes, cartes C/H/tailles, états d’intégrité/suppression/compression, limites et écriture atomique.
  - [x] Créer `src/MediaEngine/src/Formats/ImageDisk/Metadata.cpp`, `Builder.cpp` : sérialiser/restaurer la structure JSON, réconcilier l’état réel des blocs et construire les pistes Epson avec secteurs absents/invalides conservés.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, diagnostics et allowlists ; créer les catalogues de traduction ImageDisk/Epson : intégration par l’agent principal.
  - [ ] Modifier `src/Translations/Catalogs/Languages/*/MediaEngine/image-disk.json` : relire/corriger les traductions générées et vérifier paramètres/invariants.
## Batterie abandonnée sur instruction utilisateur
  - Action abandonnée : créer `build/verification-imagedisk/Reference.csproj`, `Reference.cs`, `main.cpp`, `CMakeLists.txt`, cas et manifestes : comparer les sources C# originales pour reader/writer, types de secteurs/cartes, métadonnées, reconstruction/détection Epson, annulation et conservation atomique.
  - [x] Supprimer `build/verification-imagedisk/` et les artefacts temporaires après validation.

- [x] Correction des constantes
  - [x] Modifier `src/MediaEngine/src/Formats/ImageDisk/Constants.h` et `Metadata.cpp` : nommer la longueur de la paire de caractères d'échappement JSON relevée par l'audit.

> Les batteries natives prévues dans ce document sont abandonnées sur consigne utilisateur. Ne pas les relancer. Le banc temporaire a été retiré après conservation du statut réel dans `docs/validation/media/bancs-prepares-retires.md`.

Actions natives clôturées d’après les sources courantes et les rapports existants ; les relectures encore ouvertes ne sont pas confondues avec du code manquant. Les batteries abandonnées ne sont pas présentées comme réussies. Voir `reconciliation-suivi-initial.md`.
