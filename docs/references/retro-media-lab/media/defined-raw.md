# Images brutes définies par catalogue

Sources C# : DefinedRawImageReader.cs, DefinedRawImageWriter.cs et l'entrée defined-img de MediaWritingComposition.CreateDefault.

Les deux factories reçoivent et possèdent le catalogue de définitions brutes retenues par la composition. Les dispositions et longueurs sont calculées une fois et partagées avec les opérations différées ; aucun emprunt vers une factory détruite.

Le lecteur exige un identifiant explicite et la longueur IMG correspondante. Il copie la taille utile de chaque secteur puis avance de la taille stockée img_bps, ignorant le rembourrage. Les adresses, tailles différentes, intégrité indéterminée, capacité stockée, compte logique et maximum de secteurs par piste suivent le C#. Le bloc nominal a la plus grande taille sectorielle.

Le writer valide cylindres, faces et nombre logique, puis chaque taille utile. Il écrit les données et un rembourrage nul si img_bps dépasse la taille utile. L'adapter commun impose aussi l'identifiant source égal à la cible, comme l'entrée C# defined-img. L'écriture atomique protège un fichier préexistant sur erreur et libère ses ressources par RAII.

Les quatre erreurs sont typées et traduites. Le rejet commun des lecteurs accepte uniquement les erreurs de longueur/secteur de ce lecteur ; une définition absente est une erreur de sélection, pas un rejet silencieux.

Depuis le bloc 130, les factories par défaut utilisent rawDefinitions() avec les 98 formats réservés du catalogue principal complet. Les surcharges permettent toujours de fournir un catalogue explicite.

Voir image-format-catalogue.md pour le raccordement du catalogue principal.
