# Conteneurs WOZ et reconstruction Apple partagée

WOZ1 et WOZ2 sont lus dans `Formats/Woz`. La signature, la marque binaire, le CRC32 et les chunks obligatoires INFO/TMAP/TRKS sont validés dans le même ordre que le C#. Le CRC stocké zéro ne bénéficie d’aucune exception. Les identifiants de chunks sont décodés en ASCII avec remplacement, les doublons gardent la dernière charge utile et une fin plus courte qu’un en-tête de chunk est ignorée comme dans la source.

Chaque piste examine les quatre entrées TMAP dans leur ordre d’origine, sans décoder deux fois un même descripteur pour cette piste. WOZ1 utilise les entrées fixes de 6 656 octets et leur compteur de bits ; zéro ou un compte supérieur à 53 184 donne une piste vide. WOZ2 borne les descripteurs, les blocs absolus et le nombre exact de bits avant extraction. L’API readTracks expose également les bits extraits, possédés par leur résultat.

`Formats/Apple/DecodedTracks` regroupe le décodage standard/RWTS18 et la reconstruction auparavant privée au lecteur NIB. Le score vaut 100 par numéro distinct, 10 par secteur d’intégrité explicitement valide, plus le nombre de secteurs. WOZ conserve le premier candidat en cas d’égalité. Chaque famille est évaluée séparément ; RWTS18 est retenu à partir de deux pistes crédibles. Le lecteur NIB utilise maintenant les mêmes fonctions, sans empaqueter artificiellement les données WOZ dans un conteneur NIB ni altérer leur nombre de bits.

Le writer produit WOZ1, conformément au C# : 1 à 40 pistes, maximum 53 184 bits par piste, champs INFO, TMAP quart-de-piste, octets de remplissage à un puis compteur de bits little-endian. Les chunks sont écrits après validation de quatre caractères ASCII ; le CRC32 couvre les chunks sérialisés. L’écriture par chemin C++ réutilise l’écriture atomique partagée, plus stricte que le File.WriteAllBytesAsync source sur la conservation d’une destination existante après annulation.

Les données de conteneur sont empruntées uniquement pendant la lecture ; résultats de bits et images sectorielles possèdent leurs buffers. RAII libère toutes les allocations et les ressources de fichiers sur exception ou annulation. Dix diagnostics nommés couvrent les erreurs WOZ et le builder RWTS18 vide ; les messages sont traduits dans les catalogues de langues.

Le helper ToDense autonome du builder Apple II et l’interface autonome de sélection à prédicat ne sont pas déclarés portés : leurs parcours nécessaires à la reconstruction NIB/WOZ sont couverts, mais les interfaces générales restent distinctes dans l’inventaire.
