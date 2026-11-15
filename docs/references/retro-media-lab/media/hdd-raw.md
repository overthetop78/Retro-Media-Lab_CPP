# Images brutes de disques durs

Références C# consultées en lecture seule : `Images/Formats/HardDisk/Raw/RawHardDiskFormat.cs`, `RawHardDiskReader.cs` et `RawHardDiskWriter.cs`.

Le conteneur `hdd.raw` accepte les extensions RAW, IMG, HDD, HDF et BIN. Les longueurs doivent être strictement positives et multiples de blocs logiques de 512 ou 4096 octets. La détection suit exactement l’ordre source : signature GPT au bloc 4096, puis au bloc 512, puis marque MBR. Un choix explicite permet le repli sur 512 octets. Sans choix explicite, une extension seule ne fait pas réussir la sonde.

La sonde ne valide pas la cohérence des partitions ni leurs CRC : le C# vérifie ici les signatures uniquement. Les détecteurs MFS effectuent ensuite leur propre analyse. Le message traduit décrit donc une signature, sans prétendre prouver une table cohérente.

Le reader ne charge pas l’image entière. Il ouvre une source aléatoire, vérifie à nouveau sa longueur et construit une plage stockée couvrant le disque, avec la métadonnée `logicalBlockSize`. La représentation est par blocs et le type de média est HardDisk. La lecture ne dépend pas du montage d’un système de fichiers hôte.

Le writer `hard-disk-raw` exige un média HardDisk, une représentation de blocs compatible et une couverture lisible complète. Il utilise le writer atomique et la copie par tampon existants ; les plages zéro/non allouées deviennent des octets zéro. Les plages indisponibles empêchent l’écriture. La future possède document et chemin et ne retient pas le pointeur du writer.

Les diagnostics sont structurés et traduits. Les trois rejets de lecture sont raccordés à la reconnaissance commune ; les erreurs E/S restent propagées. La composition de tous les lecteurs et le service complet de lecture ne sont pas achevés par ce conteneur.
