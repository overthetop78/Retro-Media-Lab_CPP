# Progression physique détaillée

- [x] Conserver les instantanés détaillés de lecture du C# dans le contrat matériel neutre
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/PhysicalReadingState.h` : étape, progression, cylindre/face, tentative, piste acquise, états des pistes, clé de message, paramètres et message externe ; réutiliser `DiskTrackInfo`.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/PhysicalMediaTypes.h` et `src/MediaEngine/src/PhysicalMedia/Contracts.cpp` : porter ces informations dans `PhysicalAcquisitionProgress` sans modifier les appels existants.
  - [x] Modifier `RetroMediaLab.vcxproj` et `.filters` : référencer le nouvel en-tête.
- [x] Vérifier le contrat complété
  - [x] Créer `build/verification-physical-state/CMakeLists.txt` et `main.cpp` : compiler et vérifier le transport des informations, la propriété de la piste et la compatibilité des anciens appels, sans accès matériel.
  - [x] Supprimer `build/verification-physical-state/` après utilisation.
