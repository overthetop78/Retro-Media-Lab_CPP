# Composition des codecs séquentiels

- [x] Assemblage partagé par conversion et exploration
  - [x] Créer `src/MediaEngine/src/Composition/SequentialMedia.h` et `SequentialMedia.cpp` : registres des sept décodeurs et encodeurs dans l'ordre du C#, sans dépendance App et sans inscription du codec TZX de bas niveau.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compiler et présenter les nouveaux fichiers dans la solution.
