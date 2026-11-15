# Encodage ISO MFM, CRC 16 et codes de taille

## Référence et organisation

Le bloc 23 porte `IsoMfmTrackEncoder`, le calculateur CRC 16 et les conversions de taille sectorielle. Le banc compile 21 sources C# originales inchangées. [Validation](../../../validation/media/iso-mfm-encoding.md).

`Codecs/Crc16` possède les opérations CRC non réfléchies ; `Models/Sectors/SizeCodes` possède les conversions ISO communes. `Codecs/Iso/MfmFields.cpp` construit les champs et CRC, tandis que `MfmEncoder.cpp` gère disposition logique/physique et temporalité. Le socle de piste et les primitives MFM déjà portés sont réutilisés. Aucun accès matériel, fichier ou GUI n’est ajouté.

## CRC et tailles

Le CRC accepte un polynôme et une valeur initiale explicites, avec constantes CCITT 1021, IBM 8005, FFFF et zéro. Chaque octet est injecté dans la partie haute du registre avant huit décalages non réfléchis. `compute` consomme une vue d’octets, `update` un octet supplémentaire et `append` retourne une copie suivie des deux octets fort/faible. Les allocations restent possédées et l’annulation est vérifiée dans les opérations longues.

Les codes de taille ISO 0..7 désignent 128..16 384 octets. La recherche optionnelle retourne l’absence pour les tailles non reconnues ; les conversions strictes produisent des diagnostics structurés. Ces aides sont partagées pour les autres codecs ISO à porter.

## Champs et métadonnées enregistrées

L’encodeur conserve l’identifiant `iso.mfm` et le nom technique source « ISO MFM (Atari ST / IBM PC) ». Les contraintes communes portent sur le cylindre et la face physique. Les champs CHRN enregistrés peuvent différer : RecordedCylinder/RecordedHead acceptent l’octet entier, indépendamment de la face physique. Le numéro de secteur accepte 0..255, comme dans le C#.

Sans code enregistré, le code fourni ou déduit doit correspondre à la taille du payload. Un RecordedSizeCode contourne cette validation pour préserver une taille enregistrée incohérente. Le helper propre au format retourne zéro au-delà du code sept : un code explicite supérieur à sept associé à un payload vide reste donc accepté comme dans la source, contrairement à la conversion stricte commune qui le refuse.

L’en-tête écrit trois A1 à horloge spéciale (4489 répété trois fois), FE et CHRN, puis son CRC. Une intégrité d’en-tête explicitement fausse inverse le bit faible du CRC calculé. Les données utilisent FB ou F8 selon Deleted. Deux octets de CRC enregistrés sont prioritaires sur le calcul et sur DataIntegrityValid ; si leur longueur diffère de deux, le CRC est recalculé et son bit faible inversé seulement lorsque cette intégrité est fausse. Aucun autre attribut n’est utilisé silencieusement.

## Disposition logique et physique

La disposition logique conserve les champs et gaps source : 160 cellules alternées après l’adresse, 256 après les données. La synchronisation spéciale est écrite brute ; le contenu et le CRC sont MFM. L’ordre des secteurs est celui de la requête. Le dernier intervalle sans transition est conservé dans la révolution produite.

Le mode physique est activé uniquement par `mfmPhysicalLayout == 1`. Il autorise une piste sans secteur et borne le nombre de cellules au quotient entier IndexTimeTicks / BitCellTicks. Un quotient dépassant la capacité int32 source est refusé avant allocation. Les runs de remplissage 4E et de synchronisation zéro préservent la continuité de la dernière donnée MFM. La piste commence par 80 octets 4E, puis chaque secteur emploie 12 octets zéro avant les champs, 22 octets 4E entre ID/DATA et 24 octets 4E après DATA. La fin est remplie puis tronquée exactement à la capacité ; l’intervalle terminal sans transition n’est pas ajouté.

Avec `requireCompletePhysicalTrack != 0`, tous les champs demandés doivent tenir, sinon un diagnostic est levé. Sans cette exigence, une piste trop courte peut être tronquée, même au milieu d’un champ. Une capacité nulle peut produire la même erreur de piste vide que le C# ; une piste physique vide n’est pas automatiquement une erreur si son remplissage a une capacité positive. Les données de requête ne sont pas modifiées.

## Diagnostics et suite

Cinq diagnostics de champ, taille, code et capacité physique sont dans `MediaEngine/iso-mfm-encoding.json`, générés par Argos puis relus/corrigés dans les 29 langues. Les erreurs de durée/piste communes restent dans leurs catalogues existants. Toutes les ressources locales sont libérées par RAII sur retour, erreur ou annulation.

Le décodeur ISO MFM, sa sélection PLL, puis la reconstruction SCP 1581 restent à porter. Ce bloc ne déclare pas leur validation ni celle d’un matériel réel.
