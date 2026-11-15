# Flux, fabrique de documents et adaptateurs réutilisables

- [x] Flux et fabrique
  - [x] Créer `src/MediaEngine/src/Models/Flux/Representation.h` et `Representation.cpp` : porter la représentation flux, les surfaces triées, les vues de pistes partagées et les comptages vérifiés C# sans convertir les flux en secteurs.
  - [x] Créer `src/MediaEngine/src/Images/DocumentFactory.h` et `DocumentFactory.cpp` : construire les documents de disquettes sectorielles/flux avec métadonnées, sans détection ni décodage supplémentaire.
- [x] Adaptateurs de writers
  - [x] Créer `src/MediaEngine/src/Images/WriterAdapters.h` et `WriterAdapters.cpp` : porter validations, normalisation, extraction typée secteurs/flux, délégation, cibles et liste de fichiers produits ; les futures doivent conserver document et callbacks sans conserver un pointeur vers l’adaptateur appelant.
- [x] Raccordement Commodore
  - [x] Créer `src/MediaEngine/src/Formats/CommodoreDisks/Adapters.h` et `Adapters.cpp`, modifier `Constants.h` : exposer les trois readers au contrat commun et réutiliser les writers natifs via l’adaptateur sectoriel, avec sélection explicite format/extension et conservation de la carte d’erreurs complète.
  - [x] Modifier `src/MediaEngine/src/Formats/CommodoreDisks/Writer.cpp` : expliciter la validation des nombres de pistes supportés dans le writer, présente dans le C#.
- [x] Intégration
  - [x] Modifier les diagnostics MFS, `src/MediaEngine/CMakeLists.txt`  : intégrer sources et validations typées avec constantes nommées.
  - [x] Créer `src/Translations/Catalogs/Constants/MediaFileSystems/writer-adapters.json` et les catalogues `Languages/<langue>/MediaFileSystems/writer-adapters.json` : générer avec Argos et relire les nouveaux messages.
- [x] Contrôle ciblé
  - [x] Créer `build/verification-document-adapters/CMakeLists.txt` et `main.cpp` : vérifier fabrique flux/secteurs, compteurs, adaptateur différé, validation de sortie et raccordement des trois readers Commodore ; ne pas rejouer les anciennes suites de formats.
  - [x] Supprimer `build/verification-document-adapters/` : retirer le banc et ses fichiers temporaires après consignation du résultat.
