# Préparation CUE publique

- [x] Documents et préparation des pistes
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/CuePreparation.h` et `src/MediaEngine/src/Preparation/CueDocument.cpp` : document CUE opaque, fichiers déclarés, validation binaire/audio, remplacement du fichier unique et écriture dans un flux emprunté via le parseur/sérialiseur existant.
  - [x] Créer `src/MediaEngine/src/Preparation/CueWaveFiles.cpp` : préparer les chemins via le propriétaire, extraire le PCM WAVE existant, compléter au secteur audio et publier les sorties converties et le CUE avec la transaction atomique commune ; sources conservées.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compiler et exposer la préparation.
- [x] Vérification ciblée
  - [x] Créer `build/verification-cue-preparation/CMakeLists.txt`, `main.cpp` et les petits médias temporaires : document opaque, métadonnées, disposition, flux emprunté, PCM exact et remplissage sectoriel, références répétées, source conservée, refus WAVE non audio et annulation.
  - [x] Supprimer `build/verification-cue-preparation/` après utilisation avec tous les médias et exécutables temporaires.
