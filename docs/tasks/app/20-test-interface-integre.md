# Tests matériels exécutés par le logiciel

- [x] Exécuter et restituer les vérifications depuis RML
  - [x] Modifier `src/Infrastructure/include/RetroMediaLab/Infrastructure/HardwareDiscovery.h` et `src/Infrastructure/Windows/StorageCommands.cpp` : états distincts pour chaque commande, refus de droits, rejet du périphérique et réponse invalide, sans transformer un échec en absence de capacité.
  - [x] Créer `src/Infrastructure/Hardware/DiskProbeService.h/.cpp`, `src/Infrastructure/Windows/ElevatedDiskProbe.cpp` et `src/Common/StorageProbeConstants.h` ; modifier `src/Infrastructure/Hardware/DiskProbe.cpp`, CMake et `scripts/quality/audit_literals.py` : test intégré, recours UAC limité aux interrogations en lecture seule, échange local borné, résolution des identifiants actuels et nettoyage RAII.
  - [x] Modifier `src/Launcher/main.cpp` et `src/Launcher/LauncherConstants.h` : mode auxiliaire du même logiciel chargeant uniquement Infrastructure pour la vérification élevée, sans lancer l’interface comme administrateur.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/MediaDeviceSettingsPage.cpp`, `src/Infrastructure/Hardware/HardwareConstants.h` et `src/Translations/Catalogs/{Constants,Languages/*}/App/hardware.json` : résultats par capacité, annulation UAC, erreurs et modèle identifié, dans la langue choisie.
- [x] Vérifier le parcours intégré
  - [x] Créer `build/verification-integrated-probe/` : banc GUI du bouton, tests des échanges et états, vérification UAC sur les interfaces demandées et catalogues temporaires ; aucun média embarqué.
  - [x] Supprimer `build/verification-integrated-probe/` et ses artefacts après validation Debug/Release.
