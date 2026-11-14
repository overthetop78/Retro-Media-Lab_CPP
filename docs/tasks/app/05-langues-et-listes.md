# Toutes les langues et listes déroulantes

- [x] Langues du C# et listes déroulantes normales
  - [x] Catalogue commun
    - [x] Créer `src/Translations/Languages.json`, `include/RetroMediaLab/Translations/LanguageCatalog.h` et `src/LanguageCatalog.cpp` : liste des 29 langues du C#, noms natifs invariants, codes de culture et correspondances Argos ; API commune pour les préférences et le démarrage.
    - [x] Modifier `src/Translations/CMakeLists.txt` : embarquer la liste commune dans Translations.dll et compiler son API.
    - [x] Modifier `src/App/State/PreferencesStore.cpp`, `src/App/Bootstrap/Application.cpp` et `src/App/Themes/RetroMediaLab/Views/PreferencesDialog.cpp` : utiliser les langues communes plutôt qu'une liste limitée au français/anglais.
  - [x] Catalogues traduits
    - [x] Modifier `scripts/translations/translate.py` : utiliser les codes Argos du catalogue commun et proposer la génération de toutes ses langues.
    - [x] Créer `scripts/translations/ignores/common.txt`, modifier `scripts/translations/translate.py` et supprimer tous les fichiers par langue : un seul ensemble d'invariants commun à toutes les langues, sans exception par langue.
    - [x] Créer les catalogues App sous `src/Translations/Catalogs/Languages/<langue>/App/` pour les 27 langues restantes avec Argos ; relire les fichiers et corriger les erreurs repérées.
  - [x] Style des listes
    - [x] Modifier `src/App/Themes/RetroMediaLab/RetroMediaLabStyle.h/.cpp` : listes de sélection ordinaires sous le champ, hauteur de ligne calculée depuis la police et non le rectangle de menu ; limiter le nombre de lignes visibles.
  - [x] Vérifications et nettoyage
    - [x] Créer `build/verification-languages/CMakeLists.txt` et `main.cpp` : langues disponibles, chargement de chaque DLL, réglages sauvegardés, hauteur et position des listes, sélection et conservation de la destination.
    - [x] Supprimer `build/verification-languages/` et ses artefacts après utilisation.
