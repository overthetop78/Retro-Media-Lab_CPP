# COHERENT — structures, inodes et exploration

- [x] Contrats et primitives
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Coherent.h` : entiers canoniques, pointeurs, noms, dates, validation du superbloc et lecture du volume.
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Commodore/Coherent/Constants.h`, `Internal.h`, `Codec.cpp`, `Image.cpp`, `Inode.cpp` : schéma, accès positionné aux blocs présents, limites des inodes.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h` et `FileSystemModels.h` : causes structurées des diagnostics et paramètres des commentaires techniques traduisibles.
- [x] Parcours et contenus
  - [x] Créer `src/MediaFileSystems/src/FileSystems/Commodore/Coherent/File.cpp`, `Directory.cpp`, `Reader.cpp` : pointeurs directs et trois niveaux indirects, trous, cycles/références répétées, volume et métadonnées.
- [x] Construction et messages
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, `include/RetroMediaLab/MediaFileSystems/DiagnosticKeys.h`, `src/Diagnostics.cpp`, `scripts/quality/audit_literals.py`, `scripts/translations/ignores/common.txt` et régénérer les projets Visual C++ : compilation, constantes, diagnostics et invariant COHERENT.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/coherent.json` et les 29 catalogues de langues : erreurs, avertissements et commentaires, génération Argos et relecture.
- [x] Validation
  - [x] Créer `build/verification-coherent/` : comparer les quinze sources C# originales et le C++ pour les codecs, superblocs, inodes, parcours, pointeurs indirects, fichiers dégradés et diagnostics ; vérifier annulation et bornes en Debug et Release.
  - [x] Supprimer `build/verification-coherent/` et ses artefacts après validation.
