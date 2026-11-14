# Contrôleurs disquettes et bibliothèque d'archives

- [x] Contrôleurs de disquettes
  - [x] Modifier `src/App/Hardware/HardwareConstants.h`, `HardwareSettings.h/.cpp`, `src/App/State/WorkspaceState.h`, `FloppySettingsPage.h/.cpp` et `src/Platform/Windows/HardwareDiscovery.cpp`, `src/Common/HardwareDiscoveryConstants.h` : sélecteur Greaseweazle/KryoFlux, lecteurs liés au bon contrôleur, sélections A/B ou 0/1, mémorisation du chemin DTC et détection des périphériques KryoFlux identifiés.
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/KryoFluxSettingsWidget.h/.cpp` ; modifier `src/App/CMakeLists.txt` et les catalogues hardware.json : configuration et recherche de dtc.exe, libellés et erreurs traduits, conservation des réglages lors du changement de contrôleur.
- [x] Archives multiformats
  - [x] Créer `third_party/libarchive/` et les sources des codecs nécessaires dans `third_party/` avec leurs licences, modifier `CMakeLists.txt`, `src/App/CMakeLists.txt`, `scripts/local-building/update-solution.py` et les projets générés : intégration locale de libarchive et de ses codecs, chemins de compilation et navigation Visual C++.
  - [x] Modifier `src/App/Hardware/HostToolsService.cpp` : remplacer l'extraction miniz par libarchive en conservant la vérification, les protections de chemins et le nettoyage sur exception.
  - [x] Supprimer `third_party/miniz/` et ses références de compilation, navigation et distribution après validation du remplacement.
- [x] Vérifier et nettoyer
  - [x] Créer `build/verification-controllers-archives/CMakeLists.txt` et `main.cpp` : associations Greaseweazle/KryoFlux, sélections, persistance, changement de thème/langue, archives ZIP/7z/TAR et filtres de compression disponibles, erreurs, nettoyage et rendu.
  - [x] Supprimer `build/verification-controllers-archives/` et les clones temporaires de récupération avec leurs artefacts après utilisation.

