# Codec Data General 2F

Bloc 48 — portage de DataGeneralFmTrackEncoder, DataGeneralFmDecoder, DataGeneralFmFormat, DataGeneralFmDescriptions et DataGeneralChecksum. Trente-six fichiers C# originaux sont compilés sans modification pour comparaison.

## Organisation et checksum

Codecs/DataGeneral sépare constantes, contrat, checksum, encodeur et décodeur. Le checksum n’est pas un CRC commun : pour chaque octet, le bas et le haut du mot courant sont combinés par XOR ; le haut suivant combine l’ancien bas et l’entrée. Le même calcul est appliqué une dernière fois avec l’octet zéro. Le résultat est un mot de seize bits. Une entrée vide retourne zéro. Cette itération terminale est conservée.

## Encodage

Après validation commune de la requête, cylindre et face sont validés pour le format : zéro à 127 et zéro à un. Pour chaque secteur, la taille de 512 octets est contrôlée avant le numéro de zéro à sept. Les métadonnées sectorielles et attributs ne changent pas cet encodeur.

Le premier octet d’identité combine cylindre sur sept bits et face au bit sept. Le numéro est décalé de deux bits dans le deuxième octet. En-tête et données emploient la même synchronisation FM constituée de 00 01. L’en-tête contient deux octets FM puis 64 cellules de gap. Les données comprennent 512 octets puis checksum fort/faible, suivis de 128 cellules de gap. L’ordre fourni est conservé.

## Décodage

Après reconstruction FM adaptative, toutes les positions de synchronisation sont collectées, sans sauter de cellules à ce stade. Le décodeur examine les paires adjacentes. La distance entre la fin de la synchronisation d’en-tête et le début de celle des données doit être comprise entre 32 et 256 cellules, bornes incluses.

Deux octets d’identité doivent être lisibles. Le cylindre et la face sont démasqués ; le secteur est décalé de deux bits à droite. Les deux bits bas du deuxième octet sont ignorés, comme dans le C#. Un secteur supérieur à sept rejette la paire. Les 514 octets du bloc de données doivent être lisibles ; toute paire tronquée est ignorée entièrement. Les bits d’horloge ne sont pas vérifiés séparément par le lecteur d’octets source.

Une paire complète conserve ses données même si le checksum est faux. Elle fournit un secteur avec cylindre et face réellement décodés, code de taille deux, taille 512 et intégrité de type checksum. Tags, format, CRC et intégrités séparées restent absents. Les octets décodés contiennent uniquement les charges utiles. Une structure d’en-tête de 64 cellules et une structure de données de 8 256 cellules sont ajoutées dans cet ordre.

Après réussite, les synchronisations situées avant la fin du bloc de données sont sautées dans la liste collectée. Une synchronisation exactement à cette fin reste disponible pour les paires suivantes. Aucun secteur partiel, structure de troncature ou suffixe circulaire n’est ajouté. La confiance commune utilise poids sectoriel deux et diviseur 24.

## Traductions et ressources

Quatre textes sont générés avec Argos puis relus/corrigés dans les 29 langues, avec réutilisation des formulations existantes de taille, plage et checksum. Data General et 2F sont protégés par la liste d’invariants commune. Tous les résultats et tampons sont possédés par RAII ; points d’entrée et boucles contrôlent l’annulation. Le décodeur se fournit au registre explicite ; catalogue complet et politiques de reconstruction restent distincts.
