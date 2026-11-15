# Readers Atari et détection ST

- [x] Constantes BPB partagées
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/FatBootSectorLayout.h` et modifier `src/MediaFileSystems/src/FileSystems/Fat/Constants.h` et `scripts/quality/audit_literals.py` : extraire sans les dupliquer les constantes BPB déjà portées pour les réutiliser depuis Engine via un en-tête public MFS.
- [x] Géométrie et lecture ST
  - [x] Modifier `src/MediaEngine/src/Formats/St/Geometry.h` et `Constants.h`, créer `Detection.cpp` : reprendre la preuve BPB puis les replis ordonnés 9/10/11/18 secteurs et 2/1 têtes, et construire l’image ST avec intégrité logique initiale conforme au C#.
- [x] Contrat commun des readers
  - [x] Créer `src/MediaEngine/src/Formats/Atari/Adapters.h`, `Adapters.cpp` et `Constants.h` : reprendre les capacités, préfixes, sondes, lecture bornée et construction des documents communs ST/MSA/ATR/ATX, en réutilisant les backends portés et en conservant annulation/E/S hors du rejet de sonde.
- [x] Intégration
  - [x] Modifier les diagnostics MFS, `src/MediaEngine/CMakeLists.txt` et `scripts/quality/audit_literals.py` : intégrer détection, adaptateurs, erreurs structurées et constantes.
  - [x] Créer les catalogues `src/Translations/Catalogs/Constants/MediaFileSystems/atari-readers.json` et `Languages/<langue>/MediaFileSystems/atari-readers.json` : générer avec Argos et relire les trois nouveaux messages.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-atari-readers/CMakeLists.txt` et `main.cpp` : vérifier preuve BPB/repli ST, raccordement des readers, sondes par signature/préfixe et propagation d’annulation/E/S ; aucune ancienne batterie de codecs.
  - [x] Supprimer `build/verification-atari-readers/` : retirer le banc et ses artefacts après consignation du résultat.
