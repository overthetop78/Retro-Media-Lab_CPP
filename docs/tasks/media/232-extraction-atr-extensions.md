# Extraction ATR et extensions de reconnaissance

- [x] Points d'entrée restants
  - [x] Créer `src/MediaEngine/src/Images/Conversion/AtrPayload.cpp` et modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaPreparation.h` : extraire les octets après l'en-tête ATR validé, sans réinterpréter les secteurs ni retirer le remplissage, avec sortie atomique et annulation.
  - [x] Modifier `src/MediaEngine/src/Primitives/Path.h`, `src/MediaEngine/src/Images/RecognitionContext.cpp` et `ReaderRegistry.cpp` : utiliser la règle d'extension commune pour les noms commençant par un point et les chemins associés.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : intégrer l'extraction.
- [x] Vérification du raccordement
  - [x] Créer `build/verification-atr-recognition/CMakeLists.txt` et `main.cpp` : extraction brute et remplissage, rejet avant écriture, annulation, noms à point initial et indices de fichiers associés.
  - [x] Supprimer `build/verification-atr-recognition/` et tous les médias temporaires créés par le contrôle.
