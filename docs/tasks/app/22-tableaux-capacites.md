# Tableaux détaillés des capacités matérielles

À réaliser après la vérification de disponibilité au démarrage.

- [x] Structurer les résultats par capacité
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/HardwareCapabilitiesTable.h/.cpp` et modifier `src/App/CMakeLists.txt`, `ThemeConstants.h` et `src/Infrastructure/Hardware/HardwareConstants.h` : colonnes capacité/état/détails, PASS vert, FAIL rouge et état neutre non vérifié, sans assimiler une annonce du pilote à un essai réel.
  - [x] Modifier `src/Infrastructure/include/RetroMediaLab/Infrastructure/HardwareDiscovery.h`, `src/Infrastructure/Windows/HardwareDiscovery.cpp`, `src/Common/HardwareDiscoveryConstants.h` et `src/App/State/WorkspaceState.h` : conserver les profils MMC détaillés et les capacités optiques réellement décodées ; enrichir les propriétés du contrôleur (pilote, emplacement, bus, limites de transfert, alignement, mode PIO et file de commandes), sans afficher les informations du disque raccordé. Modifier aussi `src/Infrastructure/Hardware/DiskProbeService.cpp`, `src/Infrastructure/Windows/StorageCommands.cpp` et `src/Common/StorageProbeConstants.h` pour transmettre ces propriétés.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/MediaDeviceSettingsPage.h/.cpp` et les catalogues `src/Translations/Catalogs/{Constants,Languages/*}/App/hardware.json` : tableaux optique et stockage, informations détaillées disponibles, modifier aussi `scripts/translations/ignores/common.txt` pour les termes techniques communs ; traduction des colonnes/états/détails/erreurs, mise à jour à chaud ; présenter les connexions du contrôleur sans modèle, capacité, numéro de série ou contenu du disque inséré.
- [x] Vérifier et nettoyer
  - [x] Créer `build/verification-capability-tables/` : réponses optiques et états stockage, rendu des tableaux et couleurs, absence de faux FAIL pour les tests non effectués, langues et conservation des résultats.
  - [x] Supprimer `build/verification-capability-tables/` et ses artefacts après compilation et validation.
