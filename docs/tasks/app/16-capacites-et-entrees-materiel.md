# Capacités optiques et entrées audio

- [x] Détection et interrogation matérielle
  - [x] Modifier `src/Platform/include/RetroMediaLab/Platform/HardwareDiscovery.h`, `src/Platform/Windows/HardwareDiscovery.cpp`, `src/Platform/Unix/HardwareDiscovery.cpp` et `src/Common/HardwareDiscoveryConstants.h` : états et types des entrées audio, entrées ligne débranchées/désactivées, accès au disque physique et interrogation optique MMC en lecture seule.
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/MediaDeviceSettingsPage.h/.cpp` et `src/App/Hardware/HardwareConstants.h` : bouton de test optique, résultats distincts des capacités d'extraction non vérifiées, présentation et sélection des entrées audio avec priorité aux entrées ligne, disponibilité réelle.
  - [x] Modifier `src/App/State/WorkspaceState.h` : conserver les résultats optiques de la session indépendamment des fenêtres du thème.
  - [x] Modifier `src/Translations/Catalogs/Constants/App/hardware.json`, `Languages/*/App/hardware.json` et `scripts/translations/ignores/common.txt` : libellés et erreurs du test optique, types/états audio et disque physique.
- [x] Validation et nettoyage
  - [x] Créer `build/verification-media-capabilities/CMakeLists.txt`, `main.cpp` et les catalogues de génération temporaires : parsing de réponses optiques, état des entrées audio, interface, ressources et traductions. La qualification des interfaces HDD et Zip est suivie au point 17.
  - [x] Supprimer `build/verification-media-capabilities/` et ses artefacts après validation.

