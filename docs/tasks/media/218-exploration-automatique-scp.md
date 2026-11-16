# Exploration automatique SCP

- [x] Inspection et classement
  - [x] Créer `src/MediaEngine/src/Exploration/Scp/Inspection.h`, `Inspection.cpp`, `Ranking.cpp` et `Constants.h` : inspection des systèmes de fichiers, normalisation et interprétations supplémentaires, classement stable par preuve, géométrie compatible et conservation des rejets.
- [x] Sonde et orchestration
  - [x] Créer `src/MediaEngine/src/Exploration/Scp/Probe.cpp` et `Service.cpp` : échantillonnage de 24 pistes, cinq familles et décodeurs ordonnés, exécution bornée et nettoyage RAII, événements de progression, normalisation puis classement des candidats.
  - [x] Modifier `src/MediaEngine/src/Composition/Runtime.h`, `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : partager lecteurs, registres et service d'inspection.
  - [x] Modifier `src/MediaEngine/src/Reconstruction/Scp/Composition.h` et `Composition.cpp` : exposer l'opération de décodage possédée par la composition pour éviter un second registre.
- [x] Raccordement des conversions
  - [x] Modifier `src/MediaEngine/src/Images/Conversion/Fat12Facade.cpp` et `Fat12Conversion.h` : explorer le chemin avec les politiques communes, détecter les interprétations hybrides et convertir seulement après validation.
- [x] Validation ciblée
  - [x] Créer `build/verification-scp-inspection/CMakeLists.txt` et `main.cpp` : ordres, égalités, rejets et événements avec candidats injectés, échantillonnage et normalisations, conversion FAT12 par chemin ; compiler le bloc assemblé.
  - [x] Supprimer `build/verification-scp-inspection/` et ses artefacts temporaires après utilisation.
