# Commodore DOS — lecture des volumes

## Référence et responsabilités

Le port reprend le lecteur C# `MediaFileSystems/FileSystems/Commodore/Dos/` et `Primitives/PetsciiCodec.cs`. Les 27 sources originales compilées par le banc sont inchangées. MediaEngine conserve les conteneurs D64/D71/D81 ; MediaFileSystems possède reconnaissance, répertoires, fichiers et BAM puis appelle MediaAnalysis. App ne reçoit aucune nouvelle dépendance. [Validation](../../../validation/media/commodore-dos.md).

Le lecteur est réparti entre `Layout.cpp`, `Sector.cpp`, `Directory.cpp`, `File.cpp`, `Bam.cpp` et `Reader.cpp`. Le codec PETSCII commun reste dans Primitives. Les résultats possèdent leurs contenus et diagnostics ; les sources ne sont pas retenues par le volume.

## Comportements conservés

Les identifiants de géométrie du lecteur DOS sont sensibles à la casse, comme dans le C#. Les coordonnées utilisent les géométries partagées ; le comportement source qui ramène certaines pistes au début de la première face d’un D64 est conservé. Un secteur doit contenir exactement 256 octets ; le lecteur DOS source ne rejette pas un secteur uniquement sur son indicateur d’intégrité.

L’en-tête contient la signature et le premier répertoire, avec repli sur sa position habituelle lorsque la piste indiquée est nulle. La reconnaissance examine au plus 64 secteurs distincts pour juger la plausibilité ; ce seuil ne limite pas l’exploration ultérieure. Les chaînes sont protégées contre cycles, coordonnées invalides, secteurs absents ou tronqués. Les entrées DEL sont ignorées, les types SEQ/PRG/USR/REL/CBM conservés, avec indicateurs ouvert/verrouillé et longueur d’enregistrement REL dans les attributs. L’ordre stable des noms est préservé.

Un fichier défectueux conserve ses octets déjà lus, son état de métadonnées invalide et ses avertissements. Sa taille annoncée vaut alors le nombre de blocs déclaré multiplié par 254. Les compteurs BAM sont additionnés sans plafonnement artificiel, y compris les valeurs des pistes étendues qui recouvrent des champs de nom dans la disposition source.

Une correction explicite distingue maintenant l’espace libre inconnu : lorsque le BAM est illisible, `freeSpaceKnown` vaut faux, avec zéro octet et les avertissements source. Le lecteur C# laissait le booléen par défaut à vrai. Ce champ n’est donc pas revendiqué identique au C# ; les compteurs et avertissements le sont.

## PETSCII et traductions

Le décodage s’arrête sur zéro ou A0 avant masquage ; les caractères non représentables deviennent U+FFFD. L’encodage conserve les contraintes de longueur en unités UTF-16 du C#. Les 65 536 unités sont parcourues : 91 unités sont acceptées, dont le s long U+017F ; le i sans point U+0131 est refusé. La conversion explicite évite une différence observée avec la conversion Unicode de Qt.

Les erreurs et avertissements sont des diagnostics structurés. Les commentaires de type sont des clés avec arguments invariants, résolues au moment de l’affichage. L’analyse reçoit le type invariant depuis ces commentaires structurés, afin de reconnaître un programme PRG sans extension indépendamment de la langue.

Les 20 clés de `MediaFileSystems/commodore-dos.json` sont générées avec Argos depuis en-US, relues et corrigées dans les 29 langues ; les invariants sont dans `scripts/translations/ignores/common.txt`. Aucun fichier de média n’est embarqué.

## Limites du bloc

L’écriture/migration Commodore DOS, les registres/documents et les autres reconstructions flux ne sont pas couverts par ce bloc. La présence de la primitive d’encodage PETSCII ne constitue pas un writer de volume.

La construction sectorielle a ensuite été portée au [bloc 19](commodore-dos-writer.md).
