# Cartouches, Famicom Disk System et GameCube FST

Référence : fichiers C# locaux en lecture seule, consultés le 15 novembre 2026.

Les cartouches Amstrad exposent les banques présentes parmi 32 emplacements de 16 Kio et contrôlent leur nombre déclaré. Les cartouches console exigent des banques contiguës valides ; le dépassement du stockage 32 bits reste vérifié après la lecture, comme dans le C#.

Le parseur de métadonnées entier reprend NumberStyles.None : chiffres décimaux, sans signe ni espaces, avec NULs finaux acceptés. La primitive est partagée entre les lecteurs.

Famicom Disk System conserve les fichiers lus avant une erreur sur une face ; une erreur fatale est produite uniquement lorsque toutes les faces échouent sans fichier récupéré. Les noms sont décodés en ASCII avec remplacement, puis débarrassés des espaces/NUL finaux.

GameCube sélectionne la première piste de données compatible avec les numéros de session/piste demandés, vérifie l'en-tête et les bornes FST, conserve l'ordre hiérarchique et ne charge pas les contenus supérieurs à 64 Mio. L'avertissement reste traduisible et l'entrée reste visible. CanRead ne masque pas les erreurs d'E/S ni l'annulation.

Les diagnostics opaques du document sont conservés séparément des avertissements structurés ; le nom affiché des banques Amstrad dispose d'une recette de traduction. Aucune dépendance à App n'est ajoutée.

## Provenance

- `src/RetroMediaLab.MediaFileSystems/FileSystems/Amstrad/Cartridge/AmstradCartridgeFileSystemReader.cs` : `c67be0db586be810ed652dddd402c71d4f77d91b833daa9d88047c2131a974ae` → `src/MediaFileSystems/src/FileSystems/Cartridge/Readers.cpp`
- `src/RetroMediaLab.MediaFileSystems/FileSystems/Console/Cartridge/ConsoleCartridgeFileSystemReader.cs` : `3b5b42d30e05cdcb40b874d13fd4e0ea55ee9ad78434ec684d385c98c12b45a4` → `src/MediaFileSystems/src/FileSystems/Cartridge/Readers.cpp`
- `src/RetroMediaLab.MediaFileSystems/Constants/ConsoleCartridgeMetadataConstants.cs` : `a768a363033eb16c98585a32d4cab7414ee624379acc39cc60c67f624639f339` → `src/MediaFileSystems/src/FileSystems/Cartridge/Constants.h`
- `src/RetroMediaLab.MediaFileSystems/FileSystems/Nintendo/FamicomDisk/FamicomDiskFileSystemReader.cs` : `1fc1c03d77164bf6b49481b0649d14b983a1a525869f63f460527cde80cf0416` → `src/MediaFileSystems/src/FileSystems/Nintendo/FamicomDisk/Reader.cpp`
- `src/RetroMediaLab.MediaFileSystems/FileSystems/Nintendo/FamicomDisk/FamicomDiskFileSystemConstants.cs` : `382a26451bceae69ac669d2d0f2c013fd7d0fff66cd8e14dab34c1537752349b` → `src/MediaFileSystems/src/FileSystems/Nintendo/FamicomDisk/Constants.h`
- `src/RetroMediaLab.MediaFileSystems/FileSystems/Nintendo/GameCube/GameCubeFstFileSystemReader.cs` : `36f3ea0ac334e731d4249f60b10f664a549f9c657045c38a976a3011713706e8` → `src/MediaFileSystems/src/FileSystems/Nintendo/GameCube/Reader.cpp`
- `src/RetroMediaLab.MediaFileSystems/FileSystems/Nintendo/GameCube/GameCubeFstFileSystemConstants.cs` : `bc65fd625665b6c382af2998724136b19b854da5d9b23290d26e4499403bcd6d` → `src/MediaFileSystems/src/FileSystems/Nintendo/GameCube/Constants.h`
