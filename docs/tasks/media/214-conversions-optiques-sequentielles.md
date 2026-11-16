# Conversions optiques et séquentielles

- [x] Conversions optiques
  - [x] Créer `src/MediaEngine/src/Images/Conversion/Optical.h` et `Optical.cpp` : destinations strictement acceptées par les writers optiques, lecture par le service commun, écriture sans conversion implicite et progression finale.
- [x] Raccordement séquentiel et API
  - [x] Modifier `src/MediaEngine/src/Images/Conversion/Sequential/Service.h` et `Service.cpp` : utiliser les services communs de lecture et d'écriture, avec conservation du constructeur existant.
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/SequentialConversion.h`, modifier `MediaConversion.h`, `Images/Conversion/Facade.cpp` et `Composition/Runtime.h` : exposer les conversions et destinations optiques/séquentielles en conservant les diagnostics structurés.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `.filters` : intégrer les sources et en-têtes ; compiler le bloc assemblé.
