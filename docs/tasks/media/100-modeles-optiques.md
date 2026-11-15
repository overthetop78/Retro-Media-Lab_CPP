# Représentations optiques

- [x] Contrats optiques et lecture
  - [x] Créer `src/MediaEngine/src/Models/Optical/Constants.h`, `Track.h`, `Track.cpp`, `Image.h` et `Image.cpp` : reprendre les validations C#, index, métadonnées, sessions et collections optionnelles, pistes lisibles, lectures stockées/utiles/sous-canaux et conservation des sources jusqu’à la fin des futures.
  - [x] Créer `src/MediaEngine/src/Sources/UnavailableRandomAccessData.h` et `.cpp` : porter la source de métadonnées sans octets lisibles et ses validations ; utiliser cette source pour les anciennes descriptions optiques par coordonnées.
- [x] Liaisons et intégration
  - [x] Créer `src/MediaEngine/src/Models/Optical/Internal.h` : partager l’arithmétique signée vérifiée et les lectures bornées entre validation des pistes et liaisons mécaniques.
  - [x] Créer `src/MediaEngine/src/Models/Optical/DriveBinding.cpp` et modifier `src/MediaEngine/src/Images/MediaDocument.cpp` : instrumenter pistes et sous-canaux, conserver toutes les métadonnées et éviter de relier deux fois la même mécanique.
  - [x] Modifier `src/MediaFileSystems/include/RetroMediaLab/MediaFileSystems/Diagnostics.h`, `DiagnosticKeys.h`, `src/MediaFileSystems/src/Diagnostics.cpp`, `src/MediaEngine/CMakeLists.txt` et `scripts/quality/audit_literals.py` : intégrer les sources, validations traduisibles et constantes nommées.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/optical-models.json` et les catalogues `src/Translations/Catalogs/Languages/<langue>/MediaFileSystems/optical-models.json` : traduire les nouvelles validations avec Argos et relire les messages.
- [x] Contrôle ciblé des nouveaux raccordements
  - [x] Créer `build/verification-optical-models/CMakeLists.txt` et `main.cpp` : contrôler une piste avec prégap stocké et sous-canaux espacés, les lectures différées et la liaison mécanique, sans média externe ni batterie comparative.
  - [x] Supprimer `build/verification-optical-models/` : retirer le banc et ses artefacts après consignation des résultats.
