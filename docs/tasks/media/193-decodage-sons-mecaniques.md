# Chargement réel des sons mécaniques

- [x] Banque et décodage
  - [x] Créer `src/MediaEngine/src/Audio/Mechanical/RecordingDecoder.h` et `RecordingDecoder.cpp` : décodage FLAC de la banque vers PCM flottant mono, réduction stéréo avec gains 0,5/0,5, fréquence native ou rééchantillonnée par FFmpeg embarqué, annulation et destruction systématique du processus.
  - [x] Modifier `src/MediaEngine/src/Audio/Mechanical/Constants.h` : constantes de flux FLAC, format flottant et arguments de réduction stéréo.
  - [x] Créer `src/MediaEngine/src/Audio/Mechanical/RecordedBank.h` et `RecordedBank.cpp` : construire le rendu du catalogue de production avec racine Assets/MechanicalAudio, loaders réels et durées issues du signal décodé.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compiler les nouveaux composants.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-mechanical-decoder/CMakeLists.txt`, `main.cpp` et les petits fichiers FLAC temporaires : mono/stéréo, conservation du gain, rééchantillonnage, annulation, entrée invalide, lecture du catalogue réel et cache après suppression de la source.
  - [x] Supprimer `build/verification-mechanical-decoder/` après utilisation avec tous ses médias et exécutables temporaires.
