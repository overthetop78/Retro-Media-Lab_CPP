# Codec Heathkit FM

Bloc 47 — portage de HeathkitFmTrackEncoder, HeathkitFmDecoder, HeathkitFmFormat, HeathkitFmCodec et HeathkitFmDescriptions, avec les modèles HeathkitHeader et HeathkitData. Trente-sept fichiers C# originaux sont compilés sans modification pour comparaison.

## Organisation

Codecs/Heathkit sépare constantes, contrat, records, encodeur et décodeur. Le XOR puis rotation utilise Codecs/RotatingChecksum déjà commun à NorthStar. L’inversion des bits d’un octet est extraite dans Primitives/Bits.h ; E-mu la réutilise par un alias de namespace, sans dépendre de Heathkit. Les noms techniques et paramètres restent des constantes.

## Records et encodage

EncodeRecord calcule le checksum sur les octets logiques, l’ajoute, puis inverse les bits de chaque octet. Une entrée vide est autorisée et produit un seul octet zéro. DecodeRecord inverse à nouveau les octets et sépare le dernier pour vérifier le checksum du reste. Pour une entrée vide, le C# échoue à l’accès values[^1] ; le C++ signale un diagnostic traduisible avant l’accès.

Après les validations communes de requête, l’encodeur lit l’attribut de piste volume (défaut zéro), valide volume puis cylindre entre zéro et 255. Pour chaque secteur, il contrôle d’abord les 256 octets, puis le numéro entre zéro et 255. L’attribut volume du secteur ne remplace pas celui de la piste. Les clés sont sensibles à la casse. Les métadonnées de CRC et adresses enregistrées restent ignorées par cet encodeur, conformément au C#.

L’en-tête logique contient volume/cylindre/numéro puis checksum ; les données contiennent 256 octets puis checksum. Les deux records sont chacun précédés de la marque FM de trois octets nuls suivis de BF. Ils sont suivis respectivement de gaps de 160 et 128 cellules nulles. Le codage utilise les primitives FM communes à 16 cellules par octet.

## Décodage et appairage

Le flux est reconstruit en FM adaptatif. Le lecteur d’octets extrait les bits de données à la manière du lecteur MFM source, sans validation supplémentaire des bits d’horloge. Un en-tête tronqué produit une structure de 64 cellules. Un en-tête complet de 128 cellules est conservé même si son checksum est faux.

À partir de la fin de l’en-tête, la première marque située au plus à 832 cellules est choisie, borne inclusive. Elle est appariée comme données dès qu’elle est trouvée, même si le record de données est tronqué. Cette marque ne devient donc pas un nouvel en-tête pendant le balayage principal. Les autres marques internes restent examinables. La collecte finale conserve les marques non appariées et non déjà structurées ; les recherches de positions utilisent des ensembles, remplaçant les parcours linéaires source sans changer leur résultat. Le tri des structures par position est stable.

Les octets décodés contiennent toujours volume/cylindre/numéro pour chaque en-tête lisible, puis les données si elles sont complètes. Les secteurs ont face logique zéro, taille 256 et code de taille un. L’intégrité globale est fausse si le checksum d’en-tête est faux ou si celui des données est faux ; sinon elle est indéterminée sans données, vraie avec données valides. Les données sont conservées même si leur somme est incorrecte. Les champs CRC enregistrés, tags, format et intégrités séparées restent absents comme dans la source. Les descriptions donnent séparément l’état des deux checksums.

Aucun suffixe circulaire n’est ajouté. La confiance commune utilise poids sectoriel deux et diviseur vingt. Le helper de données lit à partir de markOffset + 64 sans exiger la marque ; un offset négatif peut donc être accepté si la position calculée est valide, comme en C#.

## Ressources et traductions

Les résultats et buffers sont possédés par RAII ; les points d’entrée et boucles contrôlent l’annulation. Sept textes sont générés par Argos puis relus/corrigés dans les 29 langues, avec réutilisation des formulations Amiga/IWM déjà corrigées. Heathkit est un invariant commun. Le codec peut être fourni au registre explicite ; catalogue complet et politiques de reconstruction restent séparés.
