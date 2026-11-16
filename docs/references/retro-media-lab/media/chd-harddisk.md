# Profil disque dur CHD V5

Référence C# en lecture seule : ChdHardDiskReader, ChdHardDiskWriter et ChdHardDiskFormat, empreintes vérifiées dans l’inventaire. Le profil HDD C# exige une image autonome V5 non compressée avec une géométrie GDDD explicite ; cette restriction est conservée, même si le conteneur commun sait décoder d’autres versions/codecs.

Constants partage les champs du conteneur commun. Layout sépare le contrôle de l’en-tête, de la carte et de la chaîne de métadonnées ; la géométrie est analysée par champs numériques exacts CYLS/HEADS/SECS/BPS, sans accepter des suffixes, champs supplémentaires ou valeurs hors capacité. Les boucles de métadonnées sont refusées. La capacité et la taille d’unité doivent correspondre à la géométrie.

Reader produit un BlockImage avec plages stockées ou zéro, sans décompresser ni allouer un hunk entier. Il conserve donc les dimensions du profil HDD C#, sans imposer les limites de 10 GiB logiques/128 MiB par hunk du décodeur commun. La longueur de carte est contrôlée avant multiplication/allocation. Une plage stockée doit disposer du hunk physique entier, y compris pour le dernier hunk logique partiel. La reconnaissance examine les structures, la validation des entrées stockées appartient à la lecture du document, comme en C#.

Writer utilise la transaction atomique commune, des hunks de 512 KiB, une carte v5 brute et des métadonnées GDDD. La fin du dernier hunk est complétée par des zéros ; le SHA1 en offset 64 porte seulement sur les données logiques. Toutes les données sont écrites, même si elles sont nulles, comme le writer C#. Le format est Fixed et les propriétés version/variant/hunkSize sont conservées. Le document doit fournir une couverture complète et une géométrie exacte.

Les rejets sont raccordés au registre existant ; aucune logique matérielle ou de lecture n’est ajoutée à App. La composition globale des services reste à terminer.
