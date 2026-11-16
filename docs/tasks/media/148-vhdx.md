# VHDX autonome

- [x] Structures et lecture
  - [x] Créer `src/MediaEngine/src/Primitives/Crc32C.h`, `Crc32C.cpp`, `Crc32CConstants.h` : contrôle CRC32C partagé depuis la fonction C#.
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Vhdx/Constants.h`, `Container.h`, `Structures.h`, `Structures.cpp` : signatures, GUID binaires, CRC32C et sélection des en-têtes/tables redondants depuis le C#.
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Vhdx/Metadata.cpp`, `Reader.cpp` : métadonnées obligatoires, régions, états BAT et plages de blocs.
- [x] Écriture et raccordement
  - [x] Créer `src/MediaEngine/src/Formats/HardDisk/Vhdx/Writing.h`, `StructuresWriter.cpp`, `Writer.cpp` : variantes Fixed/Dynamic, structures redondantes, métadonnées, BAT et publication atomique positionnable.
  - [x] Modifier les diagnostics MediaEngine et `src/MediaEngine/src/Images/ReadRejection.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/vhdx.json` et `Languages/<langue>/MediaEngine/vhdx.json` : erreurs traduites et invariants communs.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : compiler et référencer les nouveaux fichiers et constantes.
- [x] Validation ciblée
  - [x] Créer `build/verification-vhdx/CMakeLists.txt` et `main.cpp` : structures et CRC indépendants, choix des copies redondantes, métadonnées, secteurs 512/4096, variantes, états BAT et rejets des profils exclus ; annulation atomique.
  - [x] Supprimer `build/verification-vhdx/` et ses fichiers synthétiques et binaires après consignation.
