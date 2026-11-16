# Images optiques Wii U et Xbox

- [x] Wii U WUD/WUX
  - [x] Créer `src/MediaEngine/src/Formats/Optical/WiiU/Constants.h`, `Container.h`, `Container.cpp` : profil WUD, en-tête WUX, index possédé/validé et source logique bornée.
  - [x] Créer `src/MediaEngine/src/Formats/Optical/WiiU/Reader.h`, `Reader.cpp` : reconnaissance, blocs logiques 2048, volume et métadonnées C#.
  - [x] Créer `src/MediaEngine/src/Formats/Optical/WiiU/Writer.cpp` : WUD complet et WUX à blocs SHA256 dédupliqués, padding et publication atomique seekable sans second temporaire.
- [x] Xbox XDVDFS
  - [x] Créer `src/MediaEngine/src/Formats/Optical/Xdvdfs/Constants.h`, `Reader.h`, `Reader.cpp` : double signature, racine bornée, piste optique 2048 et volume XDVDFS, sans déplacer l’exploration de fichiers hors MFS.
  - [x] Créer `src/MediaEngine/src/Formats/Optical/Xdvdfs/Writer.cpp` : copie atomique des secteurs de la piste acceptée par le C#.
- [x] Intégration
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, `src/MediaEngine/src/Images/ReadRejection.cpp` et créer `src/Translations/Catalogs/Constants/MediaEngine/console-optical.json`, `src/Translations/Catalogs/Languages/*/MediaEngine/console-optical.json` : erreurs traduisibles.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : compilation et constantes.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-console-optical/CMakeLists.txt`, `main.cpp` : WUD/WUX, déduplication/fin partielle/index/rejets, Xbox descriptor/racine/piste/copie, bornes et annulation atomique.
  - [x] Supprimer `build/verification-console-optical/` avec les projets, binaires et images temporaires après consignation.
