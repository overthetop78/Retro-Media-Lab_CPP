# Préférences des moteurs par backend

- [x] Préparer les choix indépendants de l’interface
  - [x] Créer `src/Infrastructure/Engines/EngineConstants.h` et `EngineSettings.h/.cpp` : choix par backend et opération, valeurs initiales du C#, conservation d’identifiants de fournisseurs futurs et absence de substitution silencieuse.
  - [x] Modifier `src/Infrastructure/CMakeLists.txt`, `src/Infrastructure/State/PreferencesStore.h/.cpp`, `src/App/State/WorkspaceState.h/.cpp` et `src/App/Bootstrap/Application.cpp` : chargement, sauvegarde et notification des préférences de moteurs, compatibilité avec les fichiers existants.
- [x] Ajouter la page Moteurs
  - [x] Créer `src/App/Themes/RetroMediaLab/Views/EngineSettingsPage.h/.cpp` et modifier `PreferencesDialog.h/.cpp`, `ThemeConstants.h`, `icons.json` et `src/App/CMakeLists.txt` : sous-onglet Greaseweazle avec Lecture/Écriture/Conversion/Explorateur et sous-onglet KryoFlux vide ; conserver la sélection à travers les changements de langue/thème.
  - [x] Créer `src/Translations/Catalogs/Constants/App/engines.json` et `Languages/*/App/engines.json` : réutiliser les libellés C# et traduire les indications de moteur interne non disponible et de fournisseur non disponible ; modifier les catalogues errors, `src/Common/ErrorKeys.h`, `scripts/quality/audit_literals.py` et les invariants communs si nécessaire.
- [x] Vérifier et nettoyer
  - [x] Créer `build/verification-engines/` : validation des choix indépendants, sauvegarde et relecture, absence de repli silencieux, onglet KryoFlux vide, langues, reconstruction du thème et contrôle visuel.
  - [x] Supprimer `build/verification-engines/` et ses artefacts après compilation et validation Debug/Release.

- [x] Retirer la phrase sur le moteur interne
  - [x] Modifier `src/App/Themes/RetroMediaLab/Views/EngineSettingsPage.h/.cpp`, `ThemeConstants.h`, `src/Infrastructure/Engines/EngineConstants.h` et les catalogues `src/Translations/Catalogs/**/App/engines.json` : supprimer le message, son contrôle et son infobulle.
