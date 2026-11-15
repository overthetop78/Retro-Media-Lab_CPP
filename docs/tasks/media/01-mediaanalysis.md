# Bloc MediaAnalysis — portage complet du classifieur C#

Le reste de MediaEngine et MediaFileSystems reste dans l’objectif global ; ce fichier décrit le premier bloc exécutable.

- [x] Contrats et données
  - [x] Créer `src/MediaAnalysis/include/RetroMediaLab/MediaAnalysis/Export.h`, `Models.h`, `Recognition.h`, `Classifier.h` et `Localization.h` : exports portables, neuf enums C#, contrats de signature/règle/résultat, entrées de classification et API sans dépendance Qt/GUI/Windows.
  - [x] Créer `src/MediaAnalysis/src/Constants/RecognitionConstants.h`, `Extensions.h`, `Signatures.h`, `Presentation.h` : constantes techniques, extensions, 72 signatures et clés/identifiants de présentation repris du C#.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/Common.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/Amiga.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/IbmPc.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/AtariCommon.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/AtariTos.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/Atari8Bit.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/AppleDos.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/ProDos.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/Macintosh.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/Lisa.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/Commodore.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/Cpm.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/BbcMicro.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/Dec.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/Msx.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
  - [x] Créer `src/MediaAnalysis/src/Catalogs/Ucsd.h` : toutes les règles C# de cette famille, dans leur ordre original, avec signatures et longueurs nommées.
- [x] Algorithmes et intégration
  - [x] Créer `src/MediaAnalysis/src/Recognition.cpp` : normalisation, règles de familles/priorités, lectures bornées des signatures fixes et recherches ordonnées dans les deux sens.
  - [x] Créer `src/MediaAnalysis/src/Classifier.cpp` : dossiers/liens, contenu absent/vide, règles prioritaires, types natifs, heuristique texte et repli fidèle au C#.
  - [x] Créer `src/MediaAnalysis/CMakeLists.txt` et modifier `CMakeLists.txt` : construire et installer `lib/MediaAnalysis.dll` sans accès direct depuis App.
  - [x] Modifier `scripts/quality/audit_literals.py` : recenser explicitement les nouveaux en-têtes de constantes, sans exclure les algorithmes de l’audit.
  - [x] Modifier `scripts/local-building/update-solution.py` : inclure les en-têtes publics et la définition d’export MediaAnalysis pour IntelliSense.
  - [x] Modifier `RetroMediaLab.vcxproj` et `RetroMediaLab.vcxproj.filters` par le générateur existant : intégrer les sources C++ à la solution.
- [x] Catalogues de traduction
  - [x] Créer `src/Translations/Catalogs/Constants/MediaAnalysis/content-types.json` et `src/Translations/Catalogs/Languages/<langue>/MediaAnalysis/content-types.json` pour les 29 langues de `src/Translations/Languages.json` : reprendre les 25 clés de présentation depuis les ressources C# existantes, sans retraduire ni embarquer de textes dans la DLL d’algorithmes.
- [x] Validation comparative
  - [x] Créer `build/verification-mediaanalysis/` : banc temporaire unique C#/C++ comparant catalogues, règles, résultats, limites et contenus générés en mémoire, sans toucher au C# source ni embarquer des médias externes.
  - [x] Supprimer `build/verification-mediaanalysis/` avec ses programmes, sources et résultats temporaires après validation ; conserver seulement le compte rendu sous docs/validation/media/.
