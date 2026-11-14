# Contrôleurs de disques et accès physique

À réaliser après les capacités optiques et les entrées audio.

- [x] Identifier les interfaces et leurs capacités
  - [x] Modifier `src/Infrastructure/include/RetroMediaLab/Infrastructure/HardwareDiscovery.h`, `src/Infrastructure/Windows/HardwareDiscovery.cpp`, `src/Infrastructure/Unix/HardwareDiscovery.cpp`, `src/Common/HardwareDiscoveryConstants.h` et `src/Infrastructure/CMakeLists.txt` : contrôleurs carte mère/PCI/PCIe/USB, rattachement des périphériques, accès physique et métadonnées des capacités réellement exposées par le pilote et le pont USB.
  - [x] Créer `src/Infrastructure/Hardware/DiskProbe.h/.cpp` : interrogation asynchrone d'une interface avec le disque raccordé choisi, test de lecture brute limité en lecture seule, nettoyage systématique.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/MediaDeviceSettingsPage.h/.cpp`, `src/Infrastructure/Hardware/HardwareConstants.h`, `src/App/State/WorkspaceState.h` et les catalogues hardware : distinguer contrôleur/interface, disque raccordé et résultats vérifiés, sans dépendre d'une lettre de volume ni d'un système de fichiers Windows.
- [x] Vérifier les interfaces branchées
  - [x] Créer `build/verification-disk-controllers/probe-disks.ps1`, `CMakeLists.txt`, `main.cpp` et les catalogues temporaires : identifier l'interface du HDD associé à O: et le Zip USB associé à P:, vérifier les accès bruts en lecture seule et les informations exposées ; ne pas conclure à une compatibilité universelle à partir d'un disque.
  - [x] Créer `build/verification-disk-controllers/elevated-hdd.ps1` et `elevated-result.json` : test administrateur avec UAC explicitement autorisé, trois secteurs au maximum sur l'identifiant exact du HDD USB associé à O:, sans écriture du disque ; le Zip est déclaré vide par l'utilisateur.
  - [x] Créer `build/verification-disk-controllers/elevated-zip.ps1` et `elevated-zip-result.json` : après insertion de la cartouche par l'utilisateur, test en lecture seule de trois secteurs maximum sur l'identifiant exact du Zip USB associé à P:, avec confirmation UAC.
  - [x] Supprimer `build/verification-disk-controllers/` et ses artefacts après utilisation.
