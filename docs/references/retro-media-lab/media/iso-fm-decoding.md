# Décodage ISO FM

## Référence et organisation

Le bloc 28 complète le décodeur ISO FM depuis 36 sources C# originales inchangées. [Validation](../../../validation/media/iso-fm-decoding.md).

`Codecs/Iso/FmRead` reconnaît les marques et vérifie les champs, horloges et CRC. `FmDecoder` appaire les champs et produit secteurs, structures et octets. `FmTiming` distingue la reconstruction adaptative et la reconstruction périodique à durée de cellule connue. Les primitives de lecture binaire, CRC 16 et confiance sont réutilisées. Aucun accès matériel/fichier ou GUI n’est ajouté.

## Marques, champs et intégrités

Seuls les motifs seize bits F57E/F56F/F56A sont reconnus, respectivement FE, FB et F8. Les octets suivants sont lus dans leurs cellules de données alternées ; l’intégrité requiert aussi que toutes les cellules d’horloge du champ, CRC compris, soient présentes à un. Une charge utile avec CRC correct mais horloge altérée reste donc invalide. Le motif spécial de la marque elle-même n’est pas soumis à cette vérification d’horloges ordinaires.

L’en-tête fournit CHRN, taille et CRC enregistré. Un champ incomplet garde ses valeurs zéro/absentes et une structure de marque de seize bits. Un code N supérieur à sept donne une taille zéro, sans lecture de payload par le décodeur. La première marque de données située après l’en-tête et avant le suivant est appairée, même lorsque ses données sont incomplètes.

L’intégrité globale est fausse si un CRC/contrôle d’horloge disponible est faux, vraie lorsque les données existent et aucun champ n’est faux, absente autrement. Les intégrités d’en-tête/données et la marque supprimée restent séparées. Contrairement au décodeur MFM source, le décodeur FM conserve les deux octets de CRC enregistrés des champs effectivement lus ; ils ne sont pas recalculés à la place des valeurs présentes.

Les segments d’octets et structures gardent un ordre stable par offset. FE est suivi des CHRN/CRC disponibles ; FB/F8 de leurs données/CRC disponibles. Une marque de données non appairée ajoute uniquement sa marque et une description dédiée. La confiance suit le C# : (deux fois le nombre de secteurs + nombre total de structures) / 18, plafonnée à un.

## Temporalité et période

Le mode adaptatif réutilise l’estimation FM et la reconstruction de transitions adaptative. Il n’ajoute pas de queue circulaire au flux ; les marques et données sont interprétées dans les cellules reconstruites disponibles.

Le mode à durée connue reconstruit les transitions sans adaptation. La période est l’arrondi au pair de IndexTimeTicks / cellTicks, conformément à Math.Round. Une période nulle/non positive ou une somme d’intervalles supérieure à l’index est refusée. Les valeurs non finies/non positives de durée, puis les contraintes communes du modèle de flux, sont également vérifiées. Les débordements int32 de période ou d’allocation sont refusés avant allocation.

Le buffer périodique comporte deux périodes plus la longueur maximale d’un champ de données ISO (16 384 octets et CRC, seize cellules par octet). À chaque phase, une cellule source existante est copiée ; les phases au-delà du flux reconstruit sont nulles. Cette règle conserve le comportement de la source, sans inventer de transition pour combler la fin d’index.

La collecte recherche les marques dans les deux premières périodes, puis produit les secteurs dont l’en-tête appartient à la première. Le prochain en-tête de la seconde période borne l’appairage et les données peuvent être lues après l’index. Les marques de données non appairées ne sont publiées que dans la première période. Leur suivi utilise l’offset exact, comme les records C# ; il n’applique pas la déduplication modulo utilisée par le décodeur MFM.

## Ressources et traductions

Buffers et résultats sont possédés ; RAII couvre succès, erreur et annulation. Les lectures, CRC, parcours d’horloges et construction périodique vérifient le jeton d’arrêt. Les accès bornés évitent toute lecture hors plage.

Les six clés de `MediaEngine/iso-fm-decoding.json` sont générées avec Argos, depuis en-US, puis relues dans les 29 langues. Les cinq descriptions identiques aux descriptions MFM hormis le nom réutilisent leurs traductions déjà corrigées en remplaçant l’invariant. Le diagnostic temporel a été corrigé pour parler de rotation et d’unités de temps, après les confusions d’Argos avec révolution politique, insectes et cellules biologiques ; les trois paramètres sont conservés.

La reconstruction sectorielle ISO, ses politiques par famille, registres/documents et conversions restent à porter. Les résultats de validation concernent les flux synthétiques, sans essai matériel.
