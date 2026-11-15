# AmigaDOS — exploration et création de volumes

## Sources et découpage

Les vingt fichiers `AmigaDos*.cs` de `src/RetroMediaLab.MediaFileSystems/FileSystems/Amiga/` ont été consultés et compilés directement dans le banc de référence. Leurs SHA-256 correspondent toujours à l’inventaire ; le dépôt C# reste inchangé.

`MediaFileSystems/AmigaDos.h` expose des valeurs et les opérations de reconnaissance, exploration et construction de secteurs. L’implémentation privée `FileSystems/Amiga/Dos/` sépare schéma/constants, codecs, racine/bitmap, répertoires, contenu, récupération et écriture. Les primitives big-endian bornées sont communes. MediaEngine conserve le conteneur ADF et sélectionne AmigaDOS, ADFS ou FAT12 avant l’analyse des entrées. App ne reçoit aucune dépendance directe supplémentaire.

## Lecture conservée du C#

Les huit variantes OFS/FFS sont reconnues, avec leurs identifiants international, directory-cache et long-names. Cela conserve le comportement existant : les variantes utilisent le parcours des en-têtes et des tables de hachage, sans nouveau parseur des blocs de cache. Les noms sont des chaînes Latin-1 avec longueur ; les noms longs sont le recours des variantes concernées lorsque le nom ordinaire est vide. Les entrées sont triées comme en C# : répertoires d’abord, puis comparaison ordinale sans casse. Les cas µ, ÿ, ß et É/é sont couverts.

La racine déclarée est essayée avant la racine conventionnelle au milieu du volume. La reconnaissance protégée sans signature exige une racine conventionnelle de somme valide et une table non vide. Une racine signée dont la somme est incorrecte reste lisible avec avertissement. L’asymétrie source entre reconnaissance et lecture d’une variante de boot non prise en charge est conservée.

Les répertoires suivent les chaînes avec détection de cycle ; les fichiers suivent les pointeurs inversés et les extensions. FFS expose les données directement ; OFS traite les en-têtes de données et leurs sommes. Une donnée absente ne devient pas artificiellement des zéros : le préfixe effectivement lu et les avertissements sont conservés. Les états de métadonnées et de données restent distincts. Le bitmap calcule l’espace libre ; comme en C#, un avertissement de bitmap ne bascule pas automatiquement `freeSpaceKnown` à faux.

La récupération exige un boot signé pris en charge et une racine indisponible. Elle sélectionne les en-têtes valides de fichiers/répertoires et reconstruit leur parenté en conservant l’ordre de découverte. Le motif de secteur indisponible est reconnu. Une récupération n’annonce pas d’espace libre connu.

Les dates utilisent l’époque UTC du 1er janvier 1978 et des ticks de 20 ms. Les bornes de minutes/ticks, les dates jusqu’à l’an 9999 et les données incomplètes sont vérifiées.

## Écriture et renforcements explicites

La création reste limitée à OFS et FFS, aux identifiants Amiga acceptés par le C# et aux blocs de 512 octets. Allocation, chaînes de hachage, extensions, sommes de métadonnées, somme de boot et bitmap sont comparés octet par octet au C#. Les plans possèdent leurs données ; la publication d’un fichier reste la responsabilité du writer atomique partagé de MediaEngine.

Les différences suivantes sont volontaires :

- les indicateurs d’intégrité enregistrés sont pris en compte et produisent des diagnostics codés ;
- les blocs de répertoire/bitmap courts sont traités sans accès hors limites ;
- la récupération et la création ont aussi une profondeur maximale de 64 ;
- les noms des entrées créées respectent la politique Latin-1 existante, sans conversion silencieuse d’un nom non représentable ; les dates invalides sont rejetées ;
- le writer refuse une géométrie hors de 4 à 4 066 blocs, limite de son unique bitmap, plutôt que dépasser son tableau ;
- l’horodatage UTC par défaut est capturé une fois par création. Un instant explicite permet une validation déterministe ;
- la réservation initiale du contenu d’un fichier est bornée par la capacité du média, même si un en-tête annonce une taille énorme.

Les vingt-trois diagnostics sont dans le domaine MediaFileSystems et les 29 langues. Argos a fourni les traductions ; la relecture a corrigé notamment des confusions entre bloc d’extension et extension de nom, migration et immigration, ainsi que des messages de checksum, récupération et contenu tronqué.

## Limites restantes

Les archives plates Amiga, codecs de flux/MFM, documents applicatifs, registres et façade générale des médias ne font pas partie de ce bloc. Les API de formats de MediaEngine restent privées. Aucun accès matériel ni émulation n’a été ajouté.

[Validation](../../../validation/media/amigados.md).
