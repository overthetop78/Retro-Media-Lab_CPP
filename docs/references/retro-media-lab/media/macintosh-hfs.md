# Macintosh HFS classique

Référence : les sept sources originales `FileSystems/Apple/Macintosh/Hfs` du projet C#, consultées en lecture seule. Le port utilise SectorImage, FileSystemVolume et FileSystemEntry existants. Les helpers Macintosh communs assurent MacRoman, dates depuis 1904, type Finder ASCII et tri ordinal sans casse ; les primitives big-endian et ForkKind ne sont pas dupliquées.

Le lecteur vérifie le MDB au bloc 2 (162 octets minimum, secteurs de 512 octets), la signature et la taille d’allocation non nulle alignée. La capacité est le nombre de blocs d’allocation multiplié par leur taille, distincte de la capacité du conteneur. Un compte libre supérieur au total produit un avertissement et un espace libre nul, comme dans la source.

Les trois extents intégrés sont parcourus en conservant les positions des secteurs manquants/courts par des zéros. Le contenu est tronqué à sa longueur logique ; le manque d’extents est signalé séparément des secteurs absents. Les données et ressources sont lues distinctement. L’entrée commune expose les données lorsqu’elles ne sont pas vides, sinon les ressources, et conserve leur longueur logique totale.

Le catalogue reprend exactement les nœuds feuilles exploitables du C#, leur taille de repli, les tables d’offsets, les alignements de clés et les bornes des records dossier/fichier. L’arborescence suit les identifiants parent/enfant avec détection des cycles sur le chemin courant, puis tri stable dossiers d’abord et noms sans casse. Les dates, types Finder et références de stockage passent par les helpers communs.

Le C# ne possède ni writer HFS ni récupération des extents supplémentaires via l’arbre overflow dans ces sept sources. Le port n’invente pas ces fonctionnalités : il expose le même lecteur classique avec avertissements d’extents insuffisants. `CatalogFormatIds` annonce AppleMacHfs, Mac400, Mac800 et Mac1440 dans le C# ; le raccordement au registre IFileSystemReader n’est pas exposé dans ce bloc et le Reader reste partiel pour ces seules métadonnées. Les six autres sources sont portées dans les composants privés Hfs/.

Les erreurs/avertissements sont structurés et traduisibles, avec distinction explicite du catalogue, des données et des ressources. Les buffers et modèles sont possédés par leurs objets ; l’annulation et les exceptions libèrent les ressources par RAII.
