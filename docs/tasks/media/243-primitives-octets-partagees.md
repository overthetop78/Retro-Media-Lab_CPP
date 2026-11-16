# Primitives d'octets partagées

- [x] Regrouper les opérations de bits et d'écriture 24 bits existantes
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/BytePrimitives.h` : constantes de bits, inversion des bits et écriture 24 bits bornée, sans dépendance GUI.
  - [x] Modifier `src/MediaFileSystems/src/Primitives/Binary.h` et `BinaryConstants.h` : référencer cette primitive au lieu de conserver une seconde implémentation.
  - [x] Modifier `src/MediaEngine/src/Primitives/Bits.h` et `BitConstants.h` : utiliser les primitives communes par les en-têtes publics adjacents, sans inclusion privée entre bibliothèques.
  - [x] Modifier `src/MediaEngine/src/Formats/Tape/Voc/Writer.cpp` et `src/MediaEngine/src/Encoding/Sequential/Amstrad/Encoder.cpp` : remplacer les assemblages locaux 24 bits par le helper commun.
  - [x] Modifier `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : référencer le nouvel en-tête.
- [x] Vérifier le déplacement du helper
  - [x] Créer `build/verification-byte-primitives/CMakeLists.txt` et `main.cpp` : inversion des bits et accès au même helper par les deux bibliothèques, sortie 24 bits et refus sans modification ; aucun codec n'est retesté.
  - [x] Supprimer `build/verification-byte-primitives/` après utilisation.

- [x] Séparer les constantes du helper pour le contrôle des littéraux
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/BytePrimitiveConstants.h`, modifier `BytePrimitives.h`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : conserver les constantes dans un en-tête dédié et les fonctions dans le helper.
