# Interprétations sectorielles partagées

- [x] Normalisation et candidats supplémentaires
  - [x] Créer `src/MediaEngine/src/Images/Recognition/Interpretations.h`, `Interpretations.cpp` et `InterpretationConstants.h` : politiques ordonnées Macintosh/MSX/Atari, candidats IBM/MSX/formats compatibles et garde des familles C# ; conserver les blocs et métadonnées.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : intégrer les sources et constantes du service réutilisable par les explorateurs.
- [x] Vérification dans le bloc FAT12
  - [x] Modifier `build/verification-fat12-reinterpretation/main.cpp` : vérifier l'ordre et les familles autorisées, le renommage MSX et la normalisation Atari depuis le BPB sans altérer les blocs.
