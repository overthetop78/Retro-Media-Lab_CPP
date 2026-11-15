# Codec Victor 9000 GCR

Bloc 42 — portage de Victor9kGcrTrackEncoder, Victor9kGcrDecoder et Victor9kGcrFormat (sommes et modèles inclus), avec réutilisation du codec GCR Commodore déjà porté. Trente-cinq sources C# originales sont compilées pour la comparaison.

## Encodage et champs

Codecs/Victor9000 sépare constantes, contrats/champs, encodeur et décodeur. Les coordonnées communes de piste sont validées par la base. Chaque secteur conserve l’ordre demandé et doit contenir 512 octets ; la taille est vérifiée avant son numéro sur un octet. La face physique n’est pas encodée et la face logique lue reste zéro, comme en C#.

L’en-tête comprend 06, cylindre, secteur, somme d’adresse réduite sur un octet, A1 et 1A. Le bloc de données comprend son préfixe nul, la charge utile et sa somme additive 16 bits faible/fort. La somme conserve les débordements sur 16 bits. Les marques sont 5555555555551111 et 5555555555551104. Les symboles GCR communs Commodore sont insérés à partir du bit 49 avec un pas de deux cellules dans la copie de marque, puis complétés par des cellules nulles selon le comportement de Write. Les gaps de 20 et 64 bits restent nuls.

L’opération addBlock valide la longueur de marque avant de modifier la cible ; elle conserve les données déjà présentes dans la cible et ajoute le nouveau bloc. Les lectures directes d’en-tête/données reprennent les règles des helpers source : elles supposent la position de marque fournie et ne vérifient pas à nouveau sa présence. Un préfixe de données différent de zéro n’est pas rejeté par le helper de lecture ; sa valeur est conservée. Le décodeur de piste recherche toujours le motif physique complet imposé.

## Décodage

La reconstruction utilise le NRZI doublé adaptatif commun. Aucun rebouclage circulaire n’est ajouté. Après une marque d’en-tête, les six octets sont lus avec le pas GCR de deux cellules. L’intégrité contrôle type, deux identifiants et somme d’adresse. Même un en-tête lisible invalide conserve ses coordonnées et ses octets dans la suite décodée.

La fin théorique d’en-tête reste au bit relatif 169, même si sa lecture a échoué. Les données sont recherchées à partir de cette fin et au plus jusqu’au bit relatif 1 568 mesuré depuis le début de marque. Les prologues trouvés sont marqués appariés avant décodage ; des données tronquées produisent une structure de marque seule. Les données lisibles conservent préfixe, checksum enregistré et contenu, même si l’intégrité est fausse. Les 512 octets de charge utile, mais ni préfixe ni checksum, sont ajoutés à la suite décodée.

Une invalidité connue de l’en-tête ou des données domine ; sans données et sans invalidité connue, l’intégrité reste indéterminée. Les secteurs ne remplissent ni tag, format, CRC enregistrés ni états d’intégrité séparés que la source laisse absents. L’ordre des structures reste données puis en-tête à chaque secteur, puis marques de données non appariées ; aucun tri final n’est ajouté. Les positions appariées utilisent un ensemble pour la recherche, comme dans le C#.

## Traduction et ressources

Les trois erreurs et quatre descriptions de MediaEngine/victor9000-gcr.json sont générées avec Argos puis relues dans les 29 langues. Les formulations déjà corrigées de tailles et d’intégrité sont réutilisées ; les marques sont représentées en hexadécimal sur deux caractères. Victor 9000 rejoint la liste commune d’invariants.

Tous les buffers, modèles et résultats sont possédés et libérés par RAII ; les opérations et helpers contrôlent l’annulation. Le décodeur utilise les contrats communs et peut être enregistré explicitement. Les politiques de reconnaissance/reconstruction Victor, les autres codecs et le catalogue complet restent à porter.
