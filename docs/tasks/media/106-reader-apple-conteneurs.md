# Reader Apple commun et routage des conteneurs

- [x] Routage et contrat commun
  - [x] Créer `src/MediaEngine/src/Formats/Apple/Adapter.h`, `Adapter.cpp` et `ContainerRouter.cpp`, modifier `Constants.h` : reprendre les capacités et préfixes Apple, la priorité 2IMG/DC42/WOZ/NIB/brut, la sonde par validation réelle et les métadonnées DC42.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h` et `src/MediaEngine/src/Diagnostics.cpp` : ajouter l’erreur structurée de routage Apple et préserver les erreurs E/S, annulation et débordement.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaEngine/apple-container.json` et `Languages/<langue>/MediaEngine/apple-container.json` : traduire et relire le message du routeur.
- [x] Intégration
  - [x] Modifier `src/MediaEngine/CMakeLists.txt` : compiler le reader et le routeur, puis régénérer les projets Visual C++.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-apple-container/CMakeLists.txt` et `main.cpp` : vérifier le routage et le contrat commun sur données mémoire, y compris métadonnées DC42 et propagation des erreurs non assimilables à un rejet de format.
  - [x] Supprimer `build/verification-apple-container/` après consignation du résultat.
