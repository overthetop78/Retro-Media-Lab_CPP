# Descripteurs de visualisation des médias

- [x] Contrats et registre
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/MediaVisualization.h` et `src/MediaEngine/src/Visualization/Descriptor.cpp` : éléments et descripteurs validés, surfaces/couches, direction et unité de progression.
  - [x] Créer `src/MediaEngine/src/Visualization/Providers.h`, `Registry.cpp` et `Facade.cpp` : sélection ordonnée des fournisseurs, préparation du document et façades publiques sans dépendance GUI ni MFS publique.
- [x] Fournisseurs
  - [x] Créer `src/MediaEngine/src/Visualization/PhysicalProviders.cpp` : pistes sectorielles et flux, prise en charge MFM Emu et sa vue logique.
  - [x] Créer `src/MediaEngine/src/Visualization/BlockProvider.cpp` et `Constants.h` : géométrie CHS, plages de blocs et volumes, longueur stockée des cartouches, lecture effective par tampon borné avec annulation.
  - [x] Créer `src/MediaEngine/src/Visualization/LinearProviders.cpp` : sessions/pistes optiques, faces/couches et voies séquentielles ordonnées par face/piste/canal.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaEngine/src/Diagnostics.cpp`, créer `src/Translations/Catalogs/Constants/MediaEngine/visualization.json` et ses fichiers dans `Languages/<langue>/MediaEngine/` : erreurs traduisibles de validation et de fournisseur absent.
  - [x] Modifier `src/MediaEngine/CMakeLists.txt`, `src/MediaEngine/src/Composition/Runtime.h`, `scripts/quality/audit_literals.py`, `RetroMediaLab.vcxproj` et `.filters` : intégrer les fournisseurs et leur composition.
- [x] Vérification ciblée
  - [x] Créer `build/verification-media-visualization/CMakeLists.txt` et `main.cpp` : validation des contrats, parcours des cinq représentations, limites 64 bits et lectures bornées, annulation, sélection du registre et préparation MFM Emu.
  - [x] Supprimer `build/verification-media-visualization/` et ses artefacts après utilisation.
