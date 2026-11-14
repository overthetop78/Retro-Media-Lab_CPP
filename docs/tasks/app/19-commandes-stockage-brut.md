# Capacités des interfaces et commandes bas niveau

- [x] Distinguer les capacités de l'interface du support de test
  - [x] Modifier `src/Infrastructure/include/RetroMediaLab/Infrastructure/HardwareDiscovery.h`, les services de test, `src/App/Themes/RetroMediaLab/Views/MediaDeviceSettingsPage.cpp` et les catalogues hardware : présenter connexion, transferts, file de commandes et accès brut vérifié ; conserver les incidents de support comme diagnostics, sans les confondre avec l'absence d'une capacité du lecteur.
- [x] Vérifier le passage de commandes en lecture seule
  - [x] Créer `src/Infrastructure/Windows/StorageCommands.cpp` et modifier le contrat HardwareDiscovery, `src/Common/HardwareDiscoveryConstants.h`, `DiskProbe.cpp` et `src/Infrastructure/CMakeLists.txt` : commandes d'identification SCSI/ATA en lecture seule, protocole du pont USB identifié, aucun changement de données ni de configuration du matériel, service indépendant du GUI.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/MediaDeviceSettingsPage.cpp`, `src/Infrastructure/Hardware/HardwareConstants.h` et les catalogues hardware : afficher les voies de commande effectivement vérifiées, indépendamment du système de fichiers de l'hôte.
- [x] Garder les résultats lisibles
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/MediaDeviceSettingsPage.cpp` : conserver la hauteur minimale des cartes de résultats dans la zone défilante, sans compression ni texte coupé.
- [x] Vérifier et nettoyer
  - [x] Créer `build/verification-storage-commands/` : sources de référence temporaires, banc, scripts UAC et résultats des commandes d'identification du pont USB demandé ; vérification des données et nettoyage des ressources.
  - [x] Supprimer `build/verification-storage-commands/` et `build/verification-disk-controllers/` avec tous leurs artefacts après validation.
