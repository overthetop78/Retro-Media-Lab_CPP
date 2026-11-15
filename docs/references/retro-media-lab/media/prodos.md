# Apple ProDOS/SOS

Référence du port : les 19 fichiers originaux de `F:/GW GUI/src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/`, consultés sans modification. Les contrats restent ceux de MediaFileSystems : SectorImage, FileSystemVolume, FileSystemEntry, MigrationPlan et SectorWritePlan. Aucun modèle parallèle n’est introduit ; App ne participe pas au décodage.

Le port regroupe les types, noms et dates dans `ProDos.h`/`Codec.cpp`, la lecture de volume dans `Reader.cpp`, les répertoires dans `Directory.cpp`, les fichiers dans `File.cpp`, le bitmap dans `Bitmap.cpp` et le formatage dans `Writer.cpp`. La validation d’en-tête réutilise AppleRecognition et les identifiants de formats proviennent du catalogue commun FormatIds.

Les fichiers seedling, sapling et tree conservent les pointeurs creux, les contenus partiellement lisibles, les limites de longueur et les diagnostics du C#. Les cycles, réutilisations de blocs de répertoire, index/données absents et bitmaps tronqués sont bornés. Les noms sont ASCII, limités à 15 caractères avec politique ProDOS originale ; leur tri conserve les répertoires en premier. La lecture SOS est sélectionnée par son identifiant sans casse ASCII.

Les dates lisent les années sur le pivot 1940–2039. L’écriture conserve le modulo 100 du C#, y compris le réencodage des années extérieures à cet intervalle. L’horodatage par défaut peut être fourni explicitement en C++ pour permettre un résultat reproductible. Les champs Timestamp invalides sont rejetés avec un diagnostic dédié ; DateTimeOffset garantissait déjà cette validité dans la source.

Le writer conserve l’allocation, les répertoires racine, le bitmap et les chaînes d’index du C#. Le nombre logique de blocs est explicite : il ne se déduit pas du produit cylindres × têtes × secteurs pour une image zonée. Les adresses fournies sont conservées. Les formats de création acceptés sont ProDOS générique, ProDOS 140 et ProDOS 800 ; SOS reste un format de lecture comme en C#.

Le writer original réserve les blocs 0 à 6 et n’écrit qu’un bitmap de 512 octets. Le port rejette donc proprement les capacités inférieures à 7 ou supérieures à 4096 blocs, au lieu de reproduire une indexation hors limites. Les parcours usuels de 280 et 1600 blocs sont conservés. Le lecteur conserve la lecture de plusieurs blocs de bitmap.

Les erreurs sont des diagnostics structurés traduisibles. Les types techniques BASIC et `ProDOS ${0}` restent dans Constants ; les autres descriptions disposent de clés de traduction. Les ressources sont possédées par leurs objets et la sortie sur exception/annulation les libère.

La lecture fonctionnelle est portée. La propriété C# `CatalogFormatIds` de ProDosFileSystemReader et son raccordement au registre IFileSystemReader ne sont pas exposés dans ce bloc ; cette source reste partielle pour ces seules métadonnées d’enregistrement (18 autres sources portées).
