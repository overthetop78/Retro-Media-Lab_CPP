# TeleDisk : enregistrements et charges utiles

Sources C# en lecture seule : `Images/Formats/Floppy/TeleDisk/Td0Layout.cs`, `Td0Format.cs`, `Td0Header.cs`, `Td0Comment.cs`, `Td0Track.cs`, `Td0Sector.cs`, `Td0Image.cs`, `Td0SectorFlags.cs`, `Td0SectorEncoding.cs`, `Td0EncodedSector.cs`, `Td0SectorEncoder.cs`, `Td0SectorDecoder.cs`, `Td0Crc16.cs` et diagnostics de `Td0Exceptions.cs`.

`Records.h` conserve les octets des en-têtes et dates, le commentaire binaire, l’ordre des pistes et secteurs, les adresses enregistrées et les données facultatives. L’absence de données est distincte d’une charge utile vide. Le modèle détaillé peut conserver l’image sectorielle logique par propriétaire partagé.

Le CRC réutilise le calculateur commun avec le polynôme TeleDisk 0xA097 et une valeur initiale nulle, sans réflexion ni XOR final. L’initialisation explicite permet le chaînage du commentaire et de son en-tête.

Le décodeur accepte données brutes, mot de deux octets répété et RLE mélangeant séquences littérales et motifs de mots répétés. Les bornes sont vérifiées avant chaque lecture de contrôle ou de motif, puis la longueur finale est comparée à la longueur attendue. Pour les données invalides dépassant cette longueur, le compteur garde la longueur réellement annoncée sans allouer l’expansion entière ; la même erreur de longueur finale est produite. Les diagnostics utilisent le numéro d’encodage comme paramètre technique.

L’encodeur reprend le choix C# : mot répété seulement si la charge est de longueur paire et tous ses mots identiques, sinon brut. Le dépassement du compteur uint16 est une erreur explicite, comme le cast checked source. Les drapeaux d’erreur CRC, marque supprimée et données indisponibles restent distincts.

Les classes de lecture du conteneur, reconstruction de son image logique, compression avancée et écriture des en-têtes ne sont pas encore portées par ce bloc. Aucune sonde TeleDisk incomplète n’est enregistrée.
