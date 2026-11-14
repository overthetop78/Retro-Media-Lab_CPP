# Inclusions, constantes et erreurs traduites

- [x] Corriger la configuration de tous les fichiers C++
  - [x] Modifier `CMakeLists.txt` et `CMakePresets.json`, créer `.vscode/c_cpp_properties.json` et `.vscode/settings.json` : exporter les commandes de compilation, relier IntelliSense au build et aux inclusions Qt/MSVC, sans masquer les diagnostics.
- [x] Retirer les données brutes des sources applicatives
  - [x] Créer les en-têtes de constantes dans `src/Common/`, `src/App/Constants/`, `src/App/Themes/RetroMediaLab/`, `src/Translations/include/RetroMediaLab/Translations/` et `src/Launcher/` ; modifier les `.h/.cpp` concernés : nommer les identifiants, clés, chemins, paramètres techniques, valeurs de présentation et valeurs par défaut, avec une séparation par responsabilité.
  - [x] Modifier les `CMakeLists.txt` concernés : rendre les constantes communes accessibles aux composants et inclure les ressources générées.
  - [x] Créer `src/App/Themes/RetroMediaLab/icons.json` et modifier `ThemeIcons.cpp` : ranger les tracés et dimensions des icônes dans les données du thème, puis les charger et valider sans coordonnées brutes dans le code.
- [x] Traduire toutes les erreurs de l'application
  - [x] Créer `src/Translations/Catalogs/Constants/App/errors.json` et ses correspondants dans les 29 langues, modifier les sources C++ : remplacer les messages anglais des exceptions par des clés stables et résoudre le message dans la langue utilisateur.
  - [x] Créer `scripts/translations/compile_errors.py` et `src/Common/Errors.h`, modifier `src/App/Bootstrap/Application.cpp`, `EntryPoint.cpp` et `src/Launcher/main.cpp` : conserver le diagnostic après nettoyage, afficher les erreurs traduites et disposer d'un catalogue embarqué de secours si les DLL de traduction ne peuvent pas être chargées.
  - [x] Modifier `scripts/translations/ignores/common.txt` et `scripts/quality/audit_literals.py` : protéger et vérifier les invariants techniques présents dans les nouvelles erreurs.
- [x] Vérifier et nettoyer
  - [x] Créer `scripts/quality/audit_literals.py` : contrôle réutilisable des valeurs et textes bruts dans les sources applicatives et de la couverture des erreurs par les catalogues.
  - [x] Créer `build/verification-constants/` avec un programme temporaire et sa configuration CMake : vérifier résolution des inclusions pour chaque unité, erreurs traduites, paramètres, nettoyage et exécution sans catalogue externe disponible.
  - [x] Supprimer `build/verification-constants/` et ses artefacts après validation.

- [x] Ajouter la solution Visual C++ et le démarrage dans la langue système
  - [x] Créer `RetroMediaLab.sln`, `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` : solution Visual C++ Debug/Release x64, arborescence des sources et commandes vers le build CMake existant, sans compiler une seconde implémentation.
  - [x] Créer `scripts/local-building/update-solution.py` et modifier `scripts/local-building/build.ps1` : actualiser les fichiers et filtres du projet Visual C++ depuis l'arborescence réelle à chaque compilation.
  - [x] Modifier `src/App/Bootstrap/Application.cpp`, `src/App/State/PreferencesStore.h/.cpp` et `src/Translations/src/LanguageCatalog.cpp` : au premier démarrage, détecter la langue d'interface du système, résoudre la langue prise en charge et l'enregistrer immédiatement ; conserver ensuite la langue sauvegardée.
  - [x] Modifier `build/verification-constants/main.cpp` : vérifier la détection de langue et la priorité de la langue sauvegardée aux démarrages suivants.
