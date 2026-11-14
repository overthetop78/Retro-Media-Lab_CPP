# Général : destination et langue

- [x] Disposer les deux blocs côte à côte
  - [x] Préférence de destination
    - [x] Modifier `src/App/State/PreferencesStore.h/.cpp` : ajouter le dossier d'images par défaut aux réglages sauvegardés et relire les anciens fichiers sans cette clé.
    - [x] Modifier `src/App/State/WorkspaceState.h/.cpp` : conserver le chemin indépendamment des vues et notifier les modifications pour leur sauvegarde.
    - [x] Modifier `src/App/Bootstrap/Application.cpp` : restaurer et sauvegarder cette préférence avec la langue et l'apparence.
  - [x] Page Général
    - [x] Modifier `src/App/Themes/RetroMediaLab/Views/PreferencesDialog.h/.cpp` : placer le bloc destination à gauche et la langue à droite ; champ de chemin et Parcourir fonctionnels ; synchroniser les saisies avant reconstruction. Ne pas ajouter le bloc des tags de conversion.
    - [x] Modifier `src/Translations/Catalogs/Constants/App/preferences.json` et les versions en-US/fr-FR : libellés du dossier et du sélecteur, génération Argos puis relecture.
  - [x] Vérifications et nettoyage
    - [x] Créer `build/verification-general/CMakeLists.txt` et `main.cpp` : disposition des blocs, conservation de saisie, sauvegarde et compatibilité des réglages précédents.
    - [x] Supprimer `build/verification-general/` et ses artefacts après utilisation.
