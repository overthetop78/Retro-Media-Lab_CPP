# Apple DiskCopy 4.2

Le port regroupe lecteur, writer et métadonnées dans `Formats/DiskCopy`, en réutilisant les géométries Apple, sondes et lecteur brut existants. Le mot privé 0x0100 est vérifié après les 84 octets d’en-tête ; les longueurs de données et tags sont lues en big-endian et bornées avant extraction.

Le checksum additionne les mots 16 bits big-endian dans un registre 32 bits puis effectue une rotation d’un bit vers la droite. Le premier tag de 12 octets est exclu du checksum des tags. Un checksum stocké nul signifie absent ; un checksum non nul incorrect produit un état d’intégrité faux, comme dans le C# actuel, sans rejeter automatiquement le conteneur.

Les charges sans tags sont confiées au lecteur brut Apple uniquement lorsqu’une sonde Macintosh ou ProDOS reconnaît leur structure. Les autres charges exigent autant de tags de 12 octets que de blocs de 512 octets, avec adressage Lisa FileWare, Macintosh zoné ou générique selon le nombre de blocs. La sélection du format conserve la priorité Macintosh, puis PREBOOT/MacWorks, puis Lisa Office. Les données et tags sont possédés par les blocs et libérés par RAII.

Le writer trie les blocs par numéro logique, vérifie leur présence, la somme de leurs tailles et l’uniformité des tags. La validation C# porte sur la somme de tailles égale à BlockCount × 512 ; elle ne rejette pas individuellement des blocs 511/513 octets compensés. Cette règle est conservée. Les noms Pascal sont tronqués à 63 octets dans le conteneur. L’écriture par chemin réutilise le mécanisme atomique partagé.

Les métadonnées de document conservent les octets du nom en Base64, les octets de format en décimal et les états valid/invalid/missing. La restauration renvoie une absence lorsqu’aucune des trois clés d’en-tête n’est fournie ; une combinaison partielle ou invalide est refusée par un diagnostic traduit.
