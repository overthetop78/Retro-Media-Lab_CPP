# Images SCP issues de secteurs

- [x] Visualisation et conversion
  - [x] Créer `src/MediaEngine/src/Visualization/SectorFluxVisualizer.h`, `SectorFluxVisualizer.cpp` : porter SectorImageFluxVisualizer avec les pistes, le nombre déclaré d’intervalles, les faces, les valeurs d’en-tête et la capacité source du C#.
  - [x] Créer `src/MediaEngine/src/Formats/Scp/SectorConverter.h`, `SectorConverter.cpp` : porter SectorImageScpConversionService, réutiliser l’encodeur physique, syntheticRevolution et l’écriture atomique SCP, avec sorties callback et chemin.
  - [x] Créer `src/MediaEngine/src/Formats/Scp/DiskTypes.h`, `DiskTypes.cpp`, `DiskTypeConstants.h`, modifier `Constants.h`, `src/MediaEngine/src/Formats/FormatIds.h`, `src/MediaEngine/CMakeLists.txt`, `scripts/quality/audit_literals.py` : catalogue de types, flags et constantes d’en-tête sans données brutes dans les services.
- [x] Vérification
  - [x] Créer `build/verification-sector-scp/Reference.csproj`, `Reference.cs`, `FormatConstants.cs`, `HeaderReference.cs`, `main.cpp`, `CMakeLists.txt`, `cases.json` : comparer les classes C# originales, types et en-têtes, pistes/flux/origines, conversion et octets écrits, erreurs et annulation ; extraire sans modification ReadHeader du lecteur C# pour le writer de référence, puis contrôler les parcours réels Debug/Release.
  - [x] Supprimer `build/verification-sector-scp/` et ses journaux : retirer le banc et tous les artefacts temporaires après validation.
