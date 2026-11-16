# Catalogue principal des formats d'images

Références C# : ImageFormatCatalog.cs (BuiltInImageFormatCatalog), ImageFormatModels.cs et IImageFormatCatalog.cs, avec les constantes employées et NormalizeCore de MediaFileExtensionFunctions.

Les 98 entrées ont été exportées en exécutant seulement le constructeur original dans un projet temporaire isolé, avec ses modèles et valeurs constantes. Les sources du projet C# sont uniquement liées en lecture ; aucun build ni fichier produit dans le dépôt source. Le constructeur a été exécuté une fois avec ses libellés anglais et une fois avec un résolveur retournant les clés. Il ne s'agit pas d'une batterie de tests C#.

Le JSON technique embarqué conserve ordre, identifiant, famille technique, extensions et extension par défaut, usage courant, extensions source compatibles, tag optionnel, facteur de forme, possibilités physiques et densité. Les contrats natifs publics sont dans ImageFormat.h. Les textes de présentation sont des clés du domaine MediaEngine ; les consommateurs les résolvent dans la langue courante, sans figer des chaînes traduites lors de la construction du catalogue.

Le constructeur C# fournit les libellés anglais de référence. Les fichiers Formats/Extension de son dossier 00-Base ne disposent pas de fichiers équivalents dans les dossiers de langues consultés ; certaines valeurs 00-Base diffèrent aussi des fallbacks du constructeur. Le port conserve ici ces fallbacks et les clés du constructeur. Noms de marques, modèles, codes et capacités purement techniques restent dans Constants : 95 clés de formats, 69 clés d'extensions et 26 familles sont invariantes. Les 24 descriptions non invariantes sont traduites ; leur erreur de chargement constitue une 25e chaîne traduite.

Le chargement natif valide les types JSON, les identifiants uniques et les valeurs d'enums. QFile et les modèles ont une durée de vie RAII. La table native est immuable et chargée une seule fois. GetCompatibleOutputs est repris par compatibleImageFormats, qui normalise la casse et le point d'extension en conservant l'ordre du catalogue. Une surcharge traite un catalogue fourni par l'appelant.

rawDefinitions() exclut maintenant l'ensemble complet de ces 98 identifiants avant le filtre des codecs bruts. Le résultat actuel contient 90 définitions. Les factories rawReader()/rawWriter() utilisent ce résultat par défaut ; les surcharges acceptant un catalogue personnalisé restent disponibles. La composition complète des autres lecteurs, writers et services reste en cours.
