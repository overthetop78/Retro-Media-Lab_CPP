# Interprétations des systèmes de fichiers

- [x] Politiques communes
  - [x] Créer `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/InterpretationConstants.h`, `Interpretation.h` et `src/Exploration/Composition/Interpretation.cpp` : crédibilité des alternatives, comptage des entrées et comparaison des preuves selon le C#.
  - [x] Modifier `src/MediaFileSystems/src/Exploration/Composition/Identity.cpp` : exposer l'identité brute et la famille de format, réutiliser ce calcul pour la déduplication du registre sans changer son ordre ni sa comparaison.
  - [x] Modifier `src/MediaFileSystems/CMakeLists.txt`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj`, `RetroMediaLab.vcxproj.filters` : compilation, constantes et solution.
  - [x] Modifier `src/MediaFileSystems/src/Exploration/Composition/Constants.h` et `include/RetroMediaLab/MediaFileSystems/InterpretationConstants.h`, `Interpretation.h` : partager et exposer les séparateurs d'identité du contrat C#.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-filesystem-interpretation/CMakeLists.txt` et `main.cpp` : seuils de crédibilité, preuve lexicographique, identité triée, distinction des familles et partage de la logique utilisée par le registre.
  - [x] Supprimer `build/verification-filesystem-interpretation/` et les sorties temporaires après utilisation.
