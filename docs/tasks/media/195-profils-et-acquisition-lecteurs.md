# Profils et acquisition des lecteurs

- [x] Profils disponibles et accès possédés
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/DriveService.h` et créer `src/MediaEngine/src/Drives/Profiles.cpp` : exposer les profils par catégorie et leur libellé, avec les alias du C# et leur ordre, sans exporter l’ancien DSP inutilisé pour ces catégories.
  - [x] Modifier `src/MediaEngine/src/Audio/Mechanical/Constants.h` : identifiants et libellés invariants des profils hérités.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/DriveMechanics.h`, `src/MediaEngine/src/Sources/DriveAccess.h` et `DriveOperation.h` : préciser qu’un début qui échoue n’acquiert rien, et ne fermer que les scopes construits avec succès.
  - [x] Modifier `src/MediaEngine/src/Drives/Mechanics.cpp` : restaurer l’état si le début d’accès échoue avant acquisition et retirer seulement l’accès acquis si la notification échoue ensuite.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : compiler et présenter le catalogue de profils.
- [x] Régression ciblée
  - [x] Créer `build/verification-drive-acquisition/CMakeLists.txt` et `main.cpp` : échec de validation avec un accès déjà actif, échec de notification avec et sans accès concurrent, scope d’opération refusé, fermeture unique sur exception, profils et alias C#.
  - [x] Supprimer `build/verification-drive-acquisition/` et ses exécutables temporaires après utilisation.
