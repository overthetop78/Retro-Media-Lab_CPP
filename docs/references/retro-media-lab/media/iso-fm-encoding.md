# Encodage ISO FM

## Référence et découpage

Le bloc 27 porte l’encodeur ISO FM à partir de 23 sources C# originales inchangées. [Validation](../../../validation/media/iso-fm-encoding.md).

`Codecs/Iso/FmFields.cpp` construit les marques, CHRN et CRC ; `FmEncoder.cpp` applique la disposition et la temporalité. Le socle de piste, les primitives FM/brutes/gap, le CRC 16 et les codes de taille sectorielle sont partagés avec les codecs précédents. Le codec conserve l’identifiant `iso.fm` et le nom source « ISO FM (simple densité) ». Le décodage reste un travail distinct.

## Champs et métadonnées

Les marques spéciales FE/FB/F8 sont émises brutes sous les motifs F57E/F56F/F56A. Les octets suivants sont FM avec une cellule d’horloge et une cellule de données par bit. Le CRC CCITT commence à FFFF et couvre la marque décodée suivie du contenu, sans préfixe A1 MFM.

Les champs cylindre/face enregistrés remplacent ceux de la requête et acceptent les valeurs 0..255, indépendamment de la face physique limitée à 0..1 par le socle commun. Le numéro de secteur est limité à 0..255. Un RecordedSizeCode est prioritaire et contourne la validation de cohérence ; sinon le code fourni ou calculé doit correspondre au payload. Comme dans le format source, un code supérieur à sept correspond à une taille zéro : il reste accepté avec un payload vide, tandis que le helper commun strict refuse ces codes lorsqu’il est appelé directement.

Une intégrité d’en-tête fausse inverse le bit faible du CRC calculé. Pour les données, exactement deux octets de RecordedErrorCheckCode sont prioritaires ; une autre longueur est ignorée et entraîne un recalcul, éventuellement altéré si DataIntegrityValid est faux. Le choix Deleted sélectionne la marque et le motif F8.

## Disposition et temporalité

En disposition logique, chaque en-tête est suivi de 160 cellules de gap et chaque champ de données de 256. Le gap commun alterne les cellules 1/0 conformément au C# ; il n’est pas remplacé par une suite de zéros ou de uns. L’ordre des secteurs de la requête est conservé et le dernier intervalle sans transition est inclus dans la révolution.

La disposition physique est activée uniquement par `fmPhysicalLayout == 1` ; les autres valeurs conservent la disposition logique. La capacité entière est IndexTimeTicks / BitCellTicks et reste bornée à int32 avant allocation. Le mode physique autorise une piste sans secteur, commence par seize octets FF et écrit six octets zéro avant chaque marque, onze octets FF entre les champs et onze après les données. La fin est remplie de FF puis tronquée à la cellule exacte, éventuellement au milieu d’un champ.

Si `requireCompletePhysicalTrack != 0`, une piste ne pouvant conserver tous les champs demandés est refusée avec le diagnostic de capacité physique partagé. Une piste physique vide n’est pas automatiquement une erreur ; sa capacité doit produire au moins une cellule. L’intervalle terminal sans transition est omis en disposition physique. Les erreurs de durée et de piste communes restent celles du socle.

## Traductions et ressources

Deux diagnostics sont dans `MediaEngine/iso-fm-encoding.json` : champs hors plage et taille incohérente. Ils sont générés par Argos depuis en-US et relus dans les 29 langues. Le message de taille identique au MFM réutilise les traductions déjà relues en remplaçant uniquement l’invariant du nom de format ; les corrections de plage conservent les trois paramètres. L’audit reconnaît explicitement FmConstants.h comme en-tête de constantes.

Tous les buffers sont possédés et libérés par RAII sur retour/exception/annulation ; les opérations longues vérifient le jeton. Aucun accès fichier, matériel ou GUI n’est ajouté. Décodeur FM, reconstruction ISO, registres et conversions restent à porter.
