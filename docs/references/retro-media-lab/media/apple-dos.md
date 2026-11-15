# Apple DOS 3.2 et 3.3

Le port MFS regroupe les classes AppleDosFileSystemReader, AppleDosVtocReader, AppleDosTrackSectorListReader, AppleDosVolumeWriter et leurs codecs/politiques dans `FileSystems/Apple/Dos`. L’API AppleDos.h utilise exclusivement les contrats existants SectorImage, FileSystemVolume, MigrationPlan et SectorWritePlan.

Le VTOC conserve la taille de 256 octets, les 13/16 secteurs, les limites de pistes déclarées, le catalogue non nul et le bitmap big-endian. Le lecteur suit les catalogues et listes T/S avec détection des cycles, vérification des adresses et conservation des fichiers partiellement accessibles. Les listes invalides, secteurs absents et tailles déclarées incohérentes produisent des diagnostics structurés. Les secteurs de données réellement disponibles sont concaténés avec leur taille observée, comme en C# ; seuls le VTOC, les catalogues et listes ont une taille obligatoirement égale à 256.

Les noms sont décodés en ASCII à bit fort, avec retrait des espaces/nuls de fin. Le tri est stable et ordinal sans casse sur les caractères ASCII décodés. Les types et attributs sont conservés ; le fichier binaire retire son en-tête adresse/longueur seulement si sa longueur est cohérente. Son adresse de chargement est réinjectée dans les attributs bruts à partir du bit 8. Les commentaires techniques du C# sont conservés et accompagnés de clés de traduction.

Le writer réserve entièrement les pistes 0 et 17, chaînant les secteurs de catalogue à rebours sur la piste 17. L’allocation parcourt les pistes par distance à la piste 16 puis par numéro, et les secteurs par numéro. Il produit au moins un secteur de données par fichier et autant de listes T/S que nécessaire. Le bitmap, le nombre de secteurs du catalogue et les offsets des listes suivent le C#.

Un plan venant d’Apple DOS conserve le type brut ; un autre système fournit un fichier binaire. Seuls les fichiers racine avec contenu sont acceptés. Le nom technique DOS-nnn représente les numéros inférieurs à 255. Sa conversion reprend byte.TryParse : signe positif, zéro négatif, espaces ASCII périphériques et nuls terminaux sont acceptés dans les trois caractères disponibles. Les nuls doivent être terminaux avant le retrait des espaces ; un nul suivi d’un espace reste invalide.

La politique des noms et le writer restent distincts : le writer ne rejette pas un nom uniquement parce que la politique le juge invalide. Il reproduit le remplacement ASCII du C# par unité UTF-16 et le bit haut ; il refuse par diagnostic une représentation dépassant les 30 octets, là où le C# produit une exception de tableau. Le chargement binaire hors 16 bits est également diagnostiqué sans troncature.

Les résultats et buffers sont possédés par RAII. Les points d’entrée C++ de lecture/écriture acceptent une annulation, sans introduire de dépendance aux modules GUI, au matériel ou à Engine. La validation élémentaire VTOC réutilise AppleRecognition déjà porté.

Le registre complet de lecteurs de systèmes de fichiers n’est pas encore porté. Ce bloc n’ajoute donc pas un branchement Apple DOS isolé au parcours Engine actuellement limité à FAT12 ; l’API MFS est prête pour le futur registre complet.
