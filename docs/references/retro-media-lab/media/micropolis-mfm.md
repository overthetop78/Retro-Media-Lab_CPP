# Codec Micropolis MFM

Bloc 45 — portage de MicropolisMfmTrackEncoder, MicropolisMfmDecoder et MicropolisMfmFormat, y compris MicropolisChecksum et MicropolisMfmRecord. Trente-quatre sources originales C# sont compilées sans modification pour la comparaison.

## Organisation et record

Codecs/Micropolis sépare constantes, contrat, records, encodage et décodage. Un record contient 275 octets : marque FF, cylindre, numéro, dix octets réservés, 256 octets utiles, checksum et cinq octets terminaux. Le checksum couvre les 268 octets après la marque et avant le checksum ; les octets réservés sont inclus et le trailer est exclu.

Le calcul conserve exactement la réduction différée du C# : avant chaque addition, soustraire 255 seulement si la valeur courante est strictement supérieure à 255. Le résultat final est converti en octet. Ce comportement diffère d’un simple modulo 255 : FF FF produit FE, tandis que FF FF 00 produit FF.

Parse exige une longueur exacte et la marque FF. Create accepte de zéro à 262 octets d’entrée, comme la capacité effective de la copie C# ; les données au-delà de 256 peuvent remplir le trailer, puis le checksum est recalculé et remplace son octet. Au-delà de 262, le C++ signale explicitement une erreur traduisible avant toute copie ; le C# levait ArgumentException par Array.Copy. L’encodeur de piste impose bien 256 octets.

Les champs data/trailer/checksum sont des instantanés. ChecksumValid recalcule sur les octets du record et compare le checksum stocké dans le champ initial, y compris si les octets sont ensuite modifiés. Le C++ possède son tampon ; le C# conserve dans Bytes l’alias du tableau fourni à Parse. Cet alias externe n’est pas reproduit : aucun appelant ne doit conserver une référence à un tampon temporaire. Les mutations du tampon possédé gardent les mêmes résultats de checksum.

## Piste

Après validation commune des coordonnées et durées, l’encodeur vérifie chaque taille puis le numéro entre zéro et 255. Il conserve l’ordre fourni. Chaque record est précédé de quarante octets nuls MFM et suivi de 128 cellules nulles ; préambule et record ont chacun leur état initial MFM. Les métadonnées sectorielles ne changent pas cet encodeur.

Le décodeur utilise la reconstruction MFM adaptative commune. La synchronisation correspond à trois octets nuls puis FF, soit 64 cellules. Le record commence 48 cellules après cette synchronisation. Les bits d’horloge ne sont pas validés séparément par le lecteur d’octets source. Une reconnaissance complète crée un secteur et une structure de 4 448 cellules ; une troncature conserve seulement une structure de 64 cellules.

Le balayage reprend 64 cellules après chaque synchronisation reconnue, sans sauter le record entier. Des synchronisations internes peuvent donc être examinées, comme dans le C#. Aucun suffixe circulaire ni tri supplémentaire n’est ajouté. La face logique est toujours zéro, même si la requête d’encodage indique un. Le code de taille vaut un.

Le mode normal fournit 256 octets utiles par secteur ; includeRecord fournit les 275 octets complets. Dans les deux modes, DecodedBytes et les descriptions restent basés sur les 256 octets utiles. Une somme incorrecte conserve le secteur et indique l’intégrité fausse. Tags, format, CRC et intégrités séparées restent absents. La confiance est calculée par le service commun avec poids sectoriel deux et diviseur 24.

## Traductions et ressources

Les cinq textes de MediaEngine/micropolis-mfm.json sont générés avec Argos puis relus/corrigés dans les 29 langues. Les formulations de taille, plage et intégrité déjà corrigées sont réutilisées ; Micropolis est un invariant commun. Les buffers et résultats sont possédés par RAII ; les points d’entrée et les boucles contrôlent l’annulation. Les deux modes peuvent être fournis au registre explicite, séparément puisqu’ils partagent le même identifiant.
