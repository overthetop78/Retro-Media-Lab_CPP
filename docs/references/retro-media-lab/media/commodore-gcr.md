# GCR Commodore 1541/1571 — encodeur et décodeur

## Source et découpage

Le bloc 21 reprend `CommodoreGcrFormat`, `CommodoreGcrDescriptions`, `CommodoreGcrDecoder` et `CommodoreGcrTrackEncoder`. Le codec 4-vers-5 et le XOR déjà portés sont réutilisés, ainsi que le socle d’encodage de pistes, NRZI et confiance. Le banc compile 36 sources C# originales, toutes inchangées. [Validation](../../../validation/media/commodore-gcr.md).

Les contrats sont dans `Codecs/Commodore/Commodore1541.h`, les constantes dans `Commodore1541Constants.h`, avec un fichier pour l’encodeur et un pour le décodeur. Ces fichiers ne dépendent pas du GUI ou du matériel ; leurs collections possèdent les données produites.

## Encodage

L’encodeur conserve l’identifiant `commodore.gcr` et le nom technique Commodore GCR. Les validations communes précèdent celles du format. Les attributs de piste `id2` et `id1` sont lus et validés dans cet ordre, avec valeurs par défaut A1 et 1A. Les clés sont sensibles à la casse. Le cylindre est ensuite limité à 39. La piste disque vient de `track` ou du calcul cylindre + 1 + face × `tracksPerSide` (35 par défaut), et doit appartenir à 1..80. Les calculs int32 non vérifiés sont reproduits par arithmétique non signée et bit_cast, sans débordement signé C++.

Chaque secteur doit avoir 256 octets et un numéro 0..255. Son ordre est conservé. La piste contient 100 cellules à un, trois cellules brutes à zéro, vingt cellules de synchronisation, un en-tête de six octets, six cellules de gap alterné, vingt cellules de synchronisation, puis marque/données/checksum et 32 cellules de gap alterné. L’en-tête contient 08, XOR des champs suivants, numéro, piste, ID2, ID1. Le champ de données contient 07, le payload et le XOR du payload seul. Les attributs des secteurs, Deleted et autres métadonnées non consommées par ce codec C# ne modifient pas cet encodage.

## Décodage et intégrité

Après reconstruction NRZI commune, toute suite d’au moins dix cellules à un devient une structure de synchronisation. Le premier octet GCR détermine en-tête, données ou marque inconnue. Une marque connue suivie d’un enregistrement incomplet reste représentée : sa marque alimente les octets décodés, son intégrité est indéterminée et sa longueur représentée inclut seulement l’octet décodable.

Chaque en-tête est associé au premier bloc de données situé strictement après sa fin et avant la synchronisation de l’en-tête suivant. La recherche par borne supérieure conserve ce critère du C#. Tous les blocs de données restent visibles dans les structures, même non appariés. Le tri final est stable : les synchronisations, puis les structures de données, puis les structures d’en-tête conservent leur priorité aux positions égales.

Le champ cylindre du secteur décodé contient la piste disque enregistrée, comme dans le C# ; il ne faut pas encore l’interpréter comme un cylindre physique à base zéro. La face logique est zéro, le code de taille un et la taille 256. L’intégrité combinée est fausse si un contrôle présent échoue, indéterminée si les données sont absentes ou tronquées, sinon vraie, y compris lorsque l’en-tête est tronqué mais les données valides : ce dernier comportement source est conservé. Les champs individuels d’intégrité restent indéterminés. La confiance utilise tous les secteurs et structures détectés : min(1, (2 × secteurs + structures) / 42).

## Descriptions, erreurs et ressources

Les descriptions de synchronisation, données et en-tête sont des recettes localisables avec états d’intégrité imbriqués ; elles ne nécessitent pas de relancer le décodage lors d’un changement de langue. Les trois erreurs propres au format et les trois descriptions sont dans `MediaEngine/commodore-gcr.json`, générées via Argos depuis en-US puis relues/corrigées dans les 29 langues. Les états d’intégrité existants sont réutilisés. Les invariants techniques sont partagés ; FormatData est également corrigé dans l’ancien catalogue coréen Commodore 900.

Les résultats et buffers sont possédés par valeur, avec nettoyage par RAII et contrôles d’annulation. Les bornes d’allocation utilisent les primitives communes. Aucun fichier ou périphérique n’est ouvert.

## Suite

La reconstruction d’images sectorielles 1541/1571 depuis les fenêtres SCP, les conversions, registres et documents restent à porter. Ce bloc valide le codec et ses fixtures synthétiques, pas une capture matérielle ni une émulation complète.
