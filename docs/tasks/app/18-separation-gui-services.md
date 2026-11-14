# App reste une interface graphique

Correction d'architecture demandée pendant le point 16, à terminer avant les tests physiques du point 17. Le C# utilise RetroMediaLab.Infrastructure ; ce nom est conservé provisoirement pour la DLL C++ de services.

- [x] Isoler les services
  - [x] Déplacer `src/App/Hardware/`, `src/App/Logging/`, `src/App/Conversion/` et `src/App/State/PreferencesStore.h/.cpp` sous `src/Infrastructure/` ; créer `InfrastructureApi.h` et `CMakeLists.txt` : DLL indépendante d'App, sans Qt Widgets, avec export de ses contrats et services.
  - [x] Déplacer les fichiers HardwareDiscovery de `src/Platform/` sous `src/Infrastructure/` ; modifier leurs contrats/imports : les périphériques appartiennent au service matériel, les adaptations graphiques restent dans Platform.
  - [x] Modifier `src/Infrastructure/Hardware/GreaseweazleDiscovery.h/.cpp` et `src/App/Themes/RetroMediaLab/Views/FloppySettingsPage.cpp` : supprimer la dépendance du service à WorkspaceState, injecter uniquement les options de journalisation nécessaires.
  - [x] Modifier `CMakeLists.txt`, `src/App/CMakeLists.txt`, `src/Platform/CMakeLists.txt`, `scripts/local-building/update-solution.py`, `scripts/quality/audit_literals.py` et les projets générés : dépendances App vers Infrastructure, distribution lib/Infrastructure.dll, aucun lien inverse.
  - [x] Déplacer `src/App/Constants/AppConstants.h` vers `src/Common/Constants/AppConstants.h` : constantes partagées nécessaires aux préférences, sans inclusion de sources App par Infrastructure.
  - [x] Créer `src/Infrastructure/Hardware/OpticalProbe.h/.cpp` et modifier `src/App/Themes/RetroMediaLab/Views/MediaDeviceSettingsPage.h/.cpp` : exécution asynchrone et nettoyage du test dans le service, GUI limitée aux commandes et à la présentation des résultats.
- [x] Vérifier la séparation
  - [x] Modifier les fichiers temporaires du point 16 pour lier les tests aux services Infrastructure, vérifier les imports DLL et l'absence de code métier matériel dans App.

- [x] Compléter la frontière entre GUI et services
  - [x] Modifier `src/Infrastructure/Hardware/HardwareSettings.h/.cpp` et les vues FloppySettingsPage, MediaDeviceSettingsPage, HostToolsSettingsWidget : appliquer les détections et l'association des interfaces dans le service, GUI limitée aux entrées et résultats.
  - [x] Modifier `src/Infrastructure/Hardware/HostToolsService.h/.cpp` et `KryoFluxSettingsWidget.cpp` : recherche de DTC, validation des exécutables et chemins gérés dans la DLL de services.
  - [x] Modifier `src/Infrastructure/Logging/LogPaths.h/.cpp` et `src/App/Themes/RetroMediaLab/Views/LogSettingsPage.cpp` : création du dossier de journaux dans le service, ouverture de la fenêtre du dossier par le GUI.
  - [x] Modifier `src/Infrastructure/State/PreferencesStore.h/.cpp` et `src/App/Bootstrap/Application.cpp` : choix du chemin de stockage des préférences dans le service.


- [x] Déployer les dépendances des services
  - [x] Modifier `scripts/local-building/build.ps1` : fournir App.dll et Infrastructure.dll à windeployqt pour inclure Qt Network dans une distribution neuve.
