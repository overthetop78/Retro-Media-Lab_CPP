# Reprise du panneau matériel

- [x] Reprendre le fonctionnement C#
  - [x] Créer `src/App/Hardware/GreaseweazleDiscovery.h/.cpp` et `HostToolsService.h/.cpp` ; modifier `HardwareConstants.h`, `src/App/CMakeLists.txt` et `scripts/quality/audit_literals.py` : isoler détection/processus et gestion des outils des vues, restauration du téléchargement et de la recherche de mise à jour C#.
  - [x] Créer `third_party/miniz/` avec les sources nécessaires et la licence officielles, modifier `CMakeLists.txt` : intégrer le lecteur ZIP et Qt Network pour télécharger et installer les outils sans script externe.
- [x] Refaire les vues
  - [x] Remplacer `src/App/Themes/RetroMediaLab/Views/HardwareSettingsPage.h/.cpp` et créer `FloppySettingsPage.h/.cpp`, `HostToolsSettingsWidget.h/.cpp`, `MediaDeviceSettingsPage.h/.cpp` : six sous-onglets, cartes de lecteurs disquettes fidèles au C#, paramètres directement sur les cartes, panneau des outils complet, sélecteurs des périphériques hôtes pour les autres familles ; supprimer le formulaire générique nom/adresse/interface et les interfaces non implémentées présentées comme utilisables.
  - [x] Modifier `src/Translations/Catalogs/Constants/App/hardware.json` et les catalogues des 29 langues : reprendre les libellés C# des lecteurs et outils, compléter seulement les diagnostics nouveaux via Argos.
- [x] Vérifier et nettoyer
  - [x] Créer `build/verification-hardware-redo/CMakeLists.txt` et `main.cpp` : cartes et commandes, ajout depuis détection, conservation/persistance, ZIP et erreurs, nettoyage des processus sur exception, rendu à 150 % et traductions.
  - [x] Supprimer `build/verification-hardware-redo/` et les sources temporaires de récupération de miniz avec tous leurs artefacts.

- [x] Mettre à jour la navigation Visual C++
  - [x] Modifier `scripts/local-building/update-solution.py`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : inclure Qt Network et miniz dans les chemins IntelliSense et afficher les sources C de la bibliothèque intégrée dans la solution.
