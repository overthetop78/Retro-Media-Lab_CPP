# Orchestration générale des conversions

- [x] Contrats et planification
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaConversion.h`, `ConversionPlanning.h`, `ConversionConstants.h` : requêtes et résultats possédés, destinations, fidélité, sélection et sorties planifiées, variables de noms.
  - [x] Créer `src/MediaEngine/src/Images/Conversion/Planner.cpp` : compatibilité du catalogue, extensions implicites/explicites, noms et variables de date/heure, collisions et niveaux de fidélité.
- [x] Service général
  - [x] Créer `src/MediaEngine/src/Images/Conversion/Service.h`, `Service.cpp`, `Facade.cpp` et modifier `Composition/Runtime.h` : découverte des destinations, conversion intermédiaire, options de bande brute, écriture et progression, accès public sans dépendance App vers MediaFileSystems.
  - [x] Modifier `Diagnostics.h`, `DiagnosticKeys.h`, `Diagnostics.cpp`, créer `Catalogs/Constants/MediaEngine/conversion-orchestration.json` et ses catalogues de langues : erreurs traduites du service et de la planification.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : intégrer le bloc.
- [x] Vérification ciblée
  - [x] Créer `build/verification-conversion-service/CMakeLists.txt` et `main.cpp` : sélection du writer et convertisseur, propagation des options et erreurs, regroupement des messages, planification et collisions ; compiler le bloc.
  - [x] Supprimer `build/verification-conversion-service/` et ses artefacts après vérification.
