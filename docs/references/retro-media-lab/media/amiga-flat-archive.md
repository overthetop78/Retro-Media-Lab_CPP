# Archives plates de ressources Amiga

## Référence et organisation

Les huit fichiers C# `FileSystems/Amiga/FlatArchive/AmigaFlatResource*.cs` sont la référence. Ils ont été compilés sans modification dans le banc et leurs huit SHA-256 correspondent à l’inventaire. Aucun fichier du dépôt C# n’a été modifié.

Le lecteur appartient à MediaFileSystems. Le contrat `AmigaFlatArchive.h` expose reconnaissance et lecture ; les fichiers privés `Constants.h`, `Directory.cpp`, `Data.cpp`, `Reader.cpp` séparent schéma, table, plages et projection. Aucun accès matériel ni dépendance directe d’App n’est ajouté. MediaEngine reste responsable du conteneur ADF. Sa sélection conserve la priorité du catalogue C# : AmigaDOS avant l’archive plate.

## Règles conservées

La table débute au bloc 2. Chaque descripteur contient un nom ASCII de 12 octets et une longueur big-endian sur 4 octets. Le premier nom doit être exactement `Reserved`. Sa longueur fixe le début des données concaténées et couvre au moins les trois premiers blocs. Les noms doivent avoir une terminaison NUL après au moins un caractère imprimable ; le remplissage suivant accepte zéro et 255. Un premier octet 255 termine le parcours. Il faut au moins deux descripteurs et la somme des longueurs ne doit pas dépasser la capacité.

La table elle-même est refusée si l’un de ses blocs est absent ou d’intégrité différente de vrai. Les ressources sont exposées dans l’ordre des descripteurs, y compris les noms dupliqués et les longueurs nulles. La taille et la référence du premier bloc sont conservées. Les plages non alignées traversent les blocs sans supposer le système de fichiers hôte.

Comme en C#, une plage de ressource manquante est remplacée par des zéros avec un avertissement contenant les numéros des blocs. Un bloc court laisse également la fin de la plage à zéro, sans ajouter un avertissement absent dans la source. L’intégrité fausse ou inconnue génère un avertissement mais ne modifie pas les octets disponibles. Les indicateurs `metadataValid=true` et `dataValid` absent restent ceux de la source : le lecteur ne les confond pas avec les anomalies physiques.

Le nom du volume est vide, l’identifiant est `amiga-flat-resource-archive`, l’espace libre est la capacité moins la somme des longueurs. Il n’y a pas de writer ni de nouvel algorithme de décompression.

## Diagnostics et limites

Les quatre erreurs/avertissements sont des codes paramétrés traduisibles. Le commentaire technique possède une clé et un domaine facultatifs dans FileSystemEntry, en plus de sa valeur de référence ; les vrais commentaires provenant des fichiers restent inchangés. Cinq textes sont disponibles dans les 29 langues. Génération Argos suivie d’une relecture et correction des confusions de vocabulaire : source/bloc, image disque, octets, archive plate et intégrité.

Les soustractions bornées évitent les dépassements dans les contrôles de plage. La limite des tableaux de la source (2 147 483 647 octets) est contrôlée avant allocation. Une table dont la longueur dépasse cette limite est reconnue comme invalide au lieu de laisser échapper le débordement d’une conversion C#. Une ressource trop longue produit l’erreur de plage codée. L’annulation est vérifiée pendant table, ressources et lecture des blocs ; la possession des buffers repose sur RAII.

Les registres/façades/documents applicatifs restent à porter. Les codecs de flux Amiga ne sont pas concernés. [Validation](../../../validation/media/amiga-flat-archive.md).
