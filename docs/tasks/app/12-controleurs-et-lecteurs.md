# Contrôleurs et lecteurs

- [x] Modèle et détection
  - [x] Créer `src/App/Hardware/HardwareConstants.h` et `HardwareSettings.h/.cpp` ; modifier `State/PreferencesStore.h/.cpp`, `State/WorkspaceState.h/.cpp`, `Bootstrap/Application.cpp`, `src/App/CMakeLists.txt` et `scripts/quality/audit_literals.py` : six catégories, lecteurs mémorisés, réglages disquettes/audio, chemins des outils, état de disponibilité et persistance indépendante des vues.
  - [x] Créer `src/Platform/include/RetroMediaLab/Platform/HardwareDiscovery.h`, `Windows/HardwareDiscovery.cpp`, `Unix/HardwareDiscovery.cpp` et `src/Common/HardwareDiscoveryConstants.h` ; modifier `src/Platform/CMakeLists.txt` : découvrir les périphériques hôtes Windows, conserver leurs identifiants, libérer les ressources natives et signaler explicitement une détection non implémentée sur les plateformes futures.
- [x] Interface
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/HardwareSettingsPage.h/.cpp` ; modifier `PreferencesDialog.h/.cpp` : onglet Contrôleurs et lecteurs, six sous-onglets, scan, ajout/sauvegarde/oubli des lecteurs, paramètres propres à chaque famille, chemin et recherche de gw.exe, sans perte de session ; modifier `RetroMediaLabStyle.cpp` pour compacter les six sous-onglets.
  - [x] Créer `src/Translations/Catalogs/Constants/App/hardware.json` et les 29 catalogues de langue correspondants : ressources C# réutilisées, nouveaux libellés traduits via Argos, erreurs traduites et invariants protégés ; compléter `scripts/translations/ignores/common.txt` avec Greaseweazle, gw.exe et PATH.
- [x] Vérification et nettoyage
  - [x] Créer `build/verification-hardware/CMakeLists.txt` et `main.cpp` : six sous-onglets, persistance, réglages, limites de lecteurs Greaseweazle, conservation des périphériques débranchés, détection et destruction des ressources, traductions et rendu avec la police réelle.
  - [x] Supprimer `build/verification-hardware/` et ses artefacts temporaires après utilisation.

La première interface de ce bloc est remplacée par [la reprise 13](13-reprise-controleurs-et-lecteurs.md).
