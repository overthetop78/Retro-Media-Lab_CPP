# Rendu et cache des sons mécaniques

- [x] Rendu indépendant du décodeur
  - [x] Créer `src/MediaEngine/src/Audio/Mechanical/RecordedRenderer.h` et `RecordedRenderer.cpp` : cache PCM mono flottant par son/fréquence, cache des durées, démarrage puis boucle, gain et arrondi C#, préparation anticipée d’un profil et durée ; chargement injecté pour séparer le décodage du mixage.
  - [x] Modifier `src/MediaEngine/src/Audio/Mechanical/Constants.h` : paramètres de rendu C# nommés.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compilation et présentation des fichiers.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-mechanical-render/CMakeLists.txt` et `main.cpp` : références PCM déterministes pour arrondi, saturation, boucle et démarrage, cache concurrent, cache des erreurs et validation des paramètres.
  - [x] Supprimer `build/verification-mechanical-render/` après utilisation.
