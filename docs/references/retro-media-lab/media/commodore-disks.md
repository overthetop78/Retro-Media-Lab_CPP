# Commodore 1541/1571/1581 — géométries et D64/D71/D81

## Source et responsabilités

Les géométries partagées sont dans MediaFileSystems, sous `Formats/Commodore/`, avec les contrats `CommodoreGeometry.h` et constantes de dimensions. Les conteneurs restent dans `MediaEngine/Formats/CommodoreDisks/`, séparés en dispositions, lecteur et writer D81. Aucun nouveau lien direct d’App vers MediaFileSystems/MediaAnalysis ; le système de fichiers Commodore DOS et son exploration restent à porter.

Le banc compile 25 sources C# originales : géométries, dispositions, lecteurs, builder zoné, erreur sectorielle, writer D81 et primitives sectorielles déjà partagées. Leurs empreintes correspondent à l’inventaire. Les modèles sectoriels et contrats de composition sont des adaptateurs de banc minimaux ; le writer C# est injecté avec une sortie mémoire. Les algorithmes comparés ne sont pas remplacés. [Validation](../../../validation/media/commodore-disks.md).

## Géométries

La 1541 accepte 35 ou 40 pistes par face, avec une numérotation des pistes à base un et des secteurs à base zéro. Les zones contiennent 21 secteurs jusqu’à la piste 17, 19 jusqu’à 24, 18 jusqu’à 30, puis 17. Les préfixes de blocs sont construits une seule fois. Une face standard contient 683 blocs et une face étendue 768, chacun de 256 octets.

La 1571 compose exactement deux faces ; tous les blocs de la première précèdent ceux de la seconde. Les helpers valident les pistes, nombres de pistes, secteurs et faces dans le même ordre que le C#. Le helper inverse 1541 conserve son acceptation de tout nombre de faces positif ; ses produits int32 non vérifiés et les fonctions sans validation sont reproduits avec arithmétique non signée et bit_cast, sans comportement C++ indéfini. Les diagnostics sont codés et traduits ; leurs maxima sont calculés sans débordement signé.

La 1581 logique possède 80 cylindres, une face et 40 blocs de 256 octets, soit 3 200 blocs. Sa vue physique MFM possède deux faces et dix secteurs de 512 octets. La conversion physique conserve l’inversion des faces `head XOR 1` du C# ; elle ne rajoute pas de validation que le helper source n’effectuait pas.

## Dispositions et reconnaissance

| Format | Pistes par face | Faces | Données | Avec carte d’erreurs |
|---|---:|---:|---:|---:|
| D64 | 35 | 1 | 174 848 | 175 531 |
| D64 | 40 | 1 | 196 608 | 197 376 |
| D71 | 35 | 2 | 349 696 | 351 062 |
| D71 | 40 | 2 | 393 216 | 394 752 |
| D81 | 80 | 1 logique | 819 200 | non prévu |

La reconnaissance repose sur la longueur exacte et, s’il est fourni, l’identifiant de format sans casse. Elle ne lit pas les données et n’exige aucune signature inexistante. Les identifiants sont `commodore.1541`, `commodore.1571` et `commodore.1581`. Les dispositions D64/D71 exposent une clé de nom et le nombre de pistes comme argument, afin de suivre les changements de langue ; leurs anciens noms français sont comparés dans le banc.

## Sources et métadonnées

L’image retient une source partagée en lecture seule. Les données de secteurs sont lues à la demande ; seule la carte d’erreurs, lorsqu’elle existe, est copiée à l’ouverture. Sa longueur est garantie par la disposition exacte et ses accès sont bornés. L’ancien diagnostic de longueur de carte ne nécessite plus un builder exposant des offsets arbitraires ; un échec réel de lecture de source est propagé. La capacité utile exclut les octets de cette carte.

L’ordre zoné se projette vers des cylindres à base zéro, la face et le numéro de secteur. Chaque octet de carte est conservé comme diagnostic, même s’il ne correspond pas à un nom d’énumération connu. L’intégrité est vraie sans carte ou pour le code un ; tous les autres codes, y compris zéro, la rendent fausse comme dans le C#. Aucun ECC, préfixe ou détail de protection supplémentaire n’est déduit de ce code.

Le D81 utilise la source sectorielle linéaire commune, avec secteurs à base zéro et intégrité vraie pour les blocs complets. Les résultats de bloc possèdent leurs octets. La lecture différée n’est pas un instantané du fichier : le propriétaire doit conserver une source cohérente pendant son utilisation. L’énumération peut s’arrêter sans lire de payload, et une erreur de lecture d’un bloc ne détruit pas la vue de l’image.

## Écriture, textes et suite

Le writer D81 valide la géométrie et l’identifiant comme le writer linéaire C#, puis exige chacun des blocs dans l’ordre logique, sans remplissage implicite. Il emploie la publication atomique commune ; une donnée manquante, un bloc court ou une annulation conserve la destination précédente. Le writer commun D64/D71, placé dans un autre dossier C#, est porté dans le [bloc 20](commodore-container-writer.md).

Cinq erreurs de géométrie, une erreur de longueur paramétrée et quatre noms de dispositions sont dans les deux domaines de traduction. Les 29 catalogues sont générés par Argos puis relus, avec correction des mots « piste », « face » et des contraintes numériques.

Les documents/registres de lecteurs, Commodore DOS, les conversions de fichiers et la reconstruction GCR 1541/1571 ou MFM 1581 restent à porter. Les octets de diagnostic sont préservés ; les noms symboliques des codes non utilisés seront raccordés avec leurs consommateurs.
