# Géométries Apple

Bloc 65 : AppleIIGeometry, MacintoshGcrGeometry, MacintoshMfmGeometry, LisaFileWareGeometry, AppleTaggedImageGeometry et AppleRawImageLayoutCatalog du C#.

`Formats/Apple/GeometryConstants.h` rassemble les dimensions, zones et permutations ; `Geometry.h/.cpp` expose les conversions d’adresses et les dispositions de capacités. Les données retournées sont immuables et les spans du catalogue référencent des tableaux de durée de vie statique. Les permutations Apple II sont vérifiées à la compilation plutôt que par une exception d’initialisation statique.

Macintosh GCR parcourt les cylindres, puis les faces, puis les secteurs : 80 cylindres, zones de 16 cylindres à 12/11/10/9/8 secteurs, une ou deux faces. Lisa FileWare parcourt une face entière avant la suivante : 1 702 blocs, 46 cylindres, deux faces, huit zones à 22/21/20/19/18/17/16/15 secteurs. Ces deux ordres restent distincts.

Le catalogue conserve exactement les interprétations du C# : D13 ; Apple II 140 Kio en ordre DOS ou ProDOS ; 400 Kio Lisa ou Macintosh ; 800 Kio Macintosh ou ProDOS ; Macintosh MFM 1,44 Mio. Il décrit des possibilités, pas une reconnaissance de système de fichiers. En particulier, le libellé Lisa associé à la capacité 400 Kio n’est pas une affirmation que cette capacité égale la géométrie physique FileWare de 1 702 blocs.

La politique Apple de reconstruction réutilise les mêmes constantes et la fonction de zones Lisa. Ses cadences et son diagnostic de cylindre restent inchangés ; ses cadences Macintosh continuent de traiter les cylindres hors plage comme dans le C# (elles ne sont pas remplacées par une validation de géométrie plus stricte).

Quatre diagnostics de géométrie sont disponibles dans Constants et les 29 langues, après génération Argos et correction des traductions techniques. Le constructeur d’erreur UnsupportedCapacity sera raccordé avec le service qui l’utilise ; il n’est pas déclaré porté dans ce bloc.

Les lecteurs, sondes de reconnaissance, conversions d’ordre sectoriel et writers Apple restent à raccorder dans les blocs suivants. Aucune responsabilité de fichiers système n’est déplacée dans App ou dans les codecs de flux.
