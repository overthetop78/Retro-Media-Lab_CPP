# COHERENT — lecture de volumes V7 Commodore 900

## Référence et responsabilités

Les quinze sources C# `FileSystems/Coherent/*.cs` ont été consultées, compilées sans modification dans le banc de référence et vérifiées par SHA-256 contre l’inventaire. Le dépôt C# est resté en lecture seule.

Le contrat `Coherent.h` expose les codecs, validation de superbloc et lecture. `MediaFileSystems/FileSystems/Commodore/Coherent/` regroupe schéma/constants, codecs, accès positionné aux blocs, inodes, contenus, répertoires et volume. Le conteneur brut zoné Commodore 900, sa reconnaissance et les codecs GCR restent de la responsabilité de MediaEngine et ne sont pas portés dans ce bloc. App ne reçoit aucun accès direct supplémentaire.

## Structures et lecture fidèle

Le bloc logique fait 512 octets. Le mode et les numéros d’inode sont little-endian ; les valeurs canoniques 32 bits ont l’ordre 2, 3, 0, 1, les pointeurs 24 bits l’ordre 1, 2, 0. Les noms ASCII remplacent les octets non ASCII par `?` et retirent seulement les zéros, espaces, LF et CR finaux. Le commentaire technique d’inode conserve sa valeur de référence et fournit sa clé, son domaine et son numéro comme paramètre traduisible.

La reconnaissance du système de fichiers reste fondée sur l’identifiant de format sans casse et le bloc de 512 octets, sans prétendre valider le superbloc à ce stade. La lecture valide les noms fixes volume/pack attendus, le nombre déclaré de blocs et la zone des inodes. Le nom de volume `noname` ou `xxxxx` devient vide. La capacité vient du nombre déclaré, l’espace libre est borné à cette capacité. Les dates Unix nulles sont absentes ; les autres valeurs uint32 couvrent jusqu’en 2106.

Chaque inode contient dix pointeurs directs et trois pointeurs indirects simple/double/triple. Chaque bloc indirect contient 128 valeurs canoniques. Les pointeurs nuls sont des trous valides, conservés sous forme de zéros. Les blocs directs absents/externes produisent leurs avertissements puis le total d’octets manquants. Les blocs indirects absents/externes/répétés produisent leurs avertissements et conservent la capacité du sous-arbre sous forme de trous. Un bloc indirect déjà rencontré reste une référence répétée même à un autre niveau. Les diagnostics sont émis dans l’ordre C#, y compris les avertissements indirects avant les données directes manquantes.

Les répertoires ignorent les numéros nuls, noms vides et entrées `.`/`..`. Pile de récursion et ensemble global distinguent cycles et secondes références. Les entrées sont triées de façon stable : répertoires d’abord, puis ordre ordinal sans casse. Les erreurs d’inode enfant deviennent des avertissements ; les erreurs racines restent bloquantes. La validité de métadonnées commune conserve le sens du C# : elle dépend des données nécessaires à l’entrée et au répertoire parent. `dataValid` reste absent. Les fichiers inconnus/périphériques sont traités comme la source, sans inventer leur interprétation.

## Présence, diagnostics et renforcements

Les blocs manquants ou courts sont distincts de blocs présents de zéros. L’implémentation C++ conserve en cache les blocs demandés plutôt que copier toute l’image et un tableau de présence dès le départ. Les octets d’un bloc consulté sont retenus pour les lectures suivantes. Cela ne constitue pas un instantané de l’ensemble d’une source fichier mutable.

Comme le C#, ce lecteur n’utilise pas les indicateurs d’intégrité des secteurs pour décider de leur présence : il ne faut pas présenter sa lecture comme une validation de checksum physique. Les blocs doivent être complets ; les octets indisponibles restent à leur position.

Les différences de robustesse sont explicites :

- la profondeur des répertoires est bornée à 64, avec avertissement et entrée invalide, pour éviter un dépassement de pile ;
- la lecture refuse une taille de bloc différente de 512, au lieu de mélanger les unités de présence du C# ;
- les additions et calculs de nombres de blocs utilisent des intermédiaires larges ; le calcul d’arrondi des tailles proches de la limite int32 ne déborde pas ;
- les erreurs de longueur canonique/pointeur, géométrie et superbloc restent codées ; l’annulation est propagée pendant le parcours et les lectures ;
- les diagnostics enfants conservent une cause structurée, traduisible séparément, au lieu d’inclure du texte d’exception brut.

Les tableaux de contenu restent soumis à la limite int32 de la source ; le portage ne fixe pas un plafond artificiel de taille du fichier à la capacité du disque, car les trous font partie de sa représentation logique. L’ensemble des buffers/blocs retenus est possédé par RAII.

Dix-huit textes sont disponibles dans les 29 langues : erreurs, avertissements, limite de profondeur et commentaire. Génération Argos puis relecture, avec correction notamment de traductions littérales d’inode enfant, de valeurs canoniques, des octets et des niveaux indirects. COHERENT est un invariant commun.

Les registres, documents, façade applicative, conteneur brut, reconstruction zonée et conversion Commodore 900 restent à porter. [Validation](../../../validation/media/coherent.md).

Le conteneur brut et la reconstruction zonée sont désormais portés dans le [bloc Commodore 900](commodore900-raw.md). Les codecs GCR, la conversion de fichiers, les registres et les documents restent en attente.
