# Conteneur VHD

Références C# en lecture seule : `Images/Formats/HardDisk/Vhd/VhdFormat.cs`, `VhdReader.cs`, `VhdWriter.cs`, constantes `HardDiskFormatConstants.cs` et enum `HardDiskImageVariant.cs`. Les empreintes des trois sources VHD correspondent à l'inventaire.

Le port utilise les représentations de blocs et les sources possédées existantes. Il sépare footer/checksum, reconstruction des plages, calcul de géométrie et writer. Le choix fixe/dynamique est extrait dans `HardDisk/Variant.cpp`, partagé avec VDI, avec variantes explicites textuelles/numériques et repli d'après les plages nulles/non allouées.

Le lecteur valide le footer final, sa signature, sa somme de contrôle, la version, la taille positive signée et l'alignement sur des secteurs de 512 octets. Les UUID du VHD utilisent l'ordre big-endian, distinct du VDI. Un VHD fixe doit avoir exactement la capacité annoncée avant le footer. Les images différentielles restent refusées sans résolveur parent, comme en C#.

Le lecteur dynamique valide la position, signature et checksum de l'en-tête, la taille de bloc et la couverture de la BAT. Les entrées inutilisées produisent des plages non allouées ; les bitmaps des blocs alloués distinguent les secteurs stockés et nuls. Le dernier bloc logique peut être partiel. Les accès doivent rester dans le fichier et les plages sont fusionnées avec calculs contrôlés. Les limites et la politique du lecteur C# sont conservées : aucun rejet nouveau des références physiques partagées ni contrôle supplémentaire de version de l'en-tête dynamique n'est inventé.

Le writer fixe copie les blocs puis écrit le footer. Le writer dynamique utilise des blocs de 2 Mio, omet les blocs entièrement nuls, écrit les bitmaps des secteurs logiques présents, complète le dernier bloc physique avec des zéros et réécrit la BAT en fin de préparation. Une même copie du footer est écrite au début et à la fin du conteneur. La signature créateur GWGU et les autres champs techniques du C# sont conservés. La date est calculée depuis l'époque UTC du 1er janvier 2000 et bornée sur uint32. La géométrie reprend les seuils 17/31/63/255 secteurs du C#.

Les deux writers utilisent la publication atomique commune et l'annulation. Les erreurs VHD de lecture sont raccordées aux rejets du registre existant. La composition complète des formats et leur ouverture depuis App restent à terminer.
