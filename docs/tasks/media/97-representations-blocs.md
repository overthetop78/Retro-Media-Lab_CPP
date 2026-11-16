# Représentations par blocs et plages de données

- [x] Contrats et possession
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/DriveMechanics.h` : reprendre les contrats C# MediaDrivePosition/State/IMediaDriveMechanics ; l'horloge et la sortie audio restent possédées par l'appelant.
  - [x] Créer `src/MediaEngine/src/Models/Blocks/BlockImage.h`, `Constants.h` et `Internal.h` : géométrie CHS validée, plages immuables, représentation liée au contrat MFS et état partagé conservé jusqu'à la fin des lectures asynchrones.
- [x] Accès et mécanique
  - [x] Créer `src/MediaEngine/src/Models/Blocks/Ranges.cpp` et `Reader.cpp` : validation/tri des plages, recherche binaire, lecture stockée/zéro/non allouée/indisponible, couverture complète et fusion ; aucune donnée inventée dans les trous.
  - [x] Créer `src/MediaEngine/src/Models/Blocks/DriveBinding.cpp` : reprise des positions CHS/logiques, découpage par piste, source instrumentée, accès directs aux plages zéro et libération systématique EndAccess sur exception.
  - [x] Créer `src/MediaEngine/src/Models/Blocks/Copy.cpp` : copies par fragments et lecture bornée de plage avec réutilisation des sources et du writer atomique existant.
- [x] Intégration
  - [x] Modifier les diagnostics publics et l'implémentation de `src/MediaFileSystems/`, `src/MediaEngine/CMakeLists.txt` et `scripts/quality/audit_literals.py` : fournir les erreurs typées du contrat de lecture de blocs, exploitables par MFS sans dépendance inverse à Engine.
  - [x] Créer les catalogues `MediaFileSystems/block-representation.json` dans `src/Translations/Catalogs/Constants/` et `Languages/<langue>/` : traduire les validations et erreurs de lecture.

Validation : compilation commune après le bloc ; un contrôle ciblé des plages et du nettoyage mécanique, sans nouvelles batteries comparatives.

- [x] Vérification bornée et retrait
  - [x] Créer `build/verification-block-representation/main.cpp` et `CMakeLists.txt` : vérifier un petit média mémoire mixte, positions CHS, conservation asynchrone et EndAccess lors d'un échec de source ou de début d'accès ; aucun comparateur C# ni média externe.
  - [x] Supprimer `build/verification-block-representation/` et ses artefacts après conservation des résultats utiles.

- [x] Source mécanique réutilisable
  - [x] Créer `src/MediaEngine/src/Sources/DriveAccess.h`, `DriveRandomAccessData.h` et `DriveRandomAccessData.cpp`, modifier `src/MediaEngine/src/Models/Blocks/Internal.h`, `DriveBinding.cpp` et `src/MediaEngine/CMakeLists.txt` : extraire la source instrumentée générique C# avec callbacks de position et de contiguïté, réutilisable ensuite par l'optique et les secteurs, sans dépendance des sources vers les modèles de blocs.

- [x] Vue de représentation pour l'application
  - [x] Créer `src/MediaEngine/include/RetroMediaLab/MediaEngine/RepresentationInfo.h` et modifier `src/MediaEngine/src/Models/Blocks/BlockImage.h` : porter les cinq types de représentation et la vue en lecture seule destinée à App, sans inclure les contrats MFS dans cette vue publique ; une même implémentation fournit les propriétés communes aux deux interfaces.

- [x] Finition des catalogues et du projet
  - [x] Modifier `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` avec le script existant : référencer les nouvelles sources, contrats et catalogues.
  - [x] Modifier les DLL de catalogues générées dans `build/debug/bin/` : compiler le nouveau catalogue par les trente cibles `RetroMediaLab.Catalog.*` après stabilisation des JSON.
- [x] Relecture linguistique restante
  - [x] Modifier les catalogues `src/Translations/Catalogs/Languages/<langue>/MediaFileSystems/block-representation.json` : relire les langues autres que le français et corriger leurs contresens.

- [x] État incrémental de l'édition de liens
  - Diagnostic : aucun `MediaEngine.ilk` ni DLL résiduelle n'est présent ; l'hypothèse d'un état incrémental ancien n'est pas confirmée, aucune suppression n'est exécutée.
  - [x] Modifier `src/MediaEngine/include/RetroMediaLab/MediaEngine/Export.h` et `RepresentationInfo.h` : appliquer `novtable` à l'interface pure sous MSVC uniquement, pour éviter l'émission de la table abstraite rejetée par LINK ; conserver une déclaration portable sur les autres compilateurs.

- [x] Nettoyage après acquisition réelle de l'accès
  - [x] Modifier `src/MediaEngine/src/Sources/DriveAccess.h`, `DriveRandomAccessData.cpp` et `src/MediaEngine/src/Models/Blocks/Reader.cpp` : calculer la position avant acquisition, commencer l'accès dans le garde RAII et nettoyer un BeginRead/BeginWrite partiellement échoué ; ne pas terminer un accès si le callback de position a échoué avant son début.

Relecture achevée le 17 novembre 2026 : voir docs/validation/media/relecture-blocs-documents.md.
