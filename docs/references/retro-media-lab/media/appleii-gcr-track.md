# Codec de piste Apple II GCR

Bloc 37 — sources C# originales en lecture seule : AppleIIGcrTrackEncoder, AppleIIGcrDecoder et paramètres restants de AppleIIGcrFormat. Les primitives restent séparées dans Address, SixAndTwo et FiveAndThree ; Encoder/Decoder utilisent les contrats communs et le registre explicite.

## Encodage

Une piste de 13 secteurs choisit 5-and-3 ; 16 secteurs choisissent 6-and-2. L’attribut sectorsPerTrack permet une piste partielle. Une piste de 13/16 secteurs dont le nombre contredit cet attribut est refusée, comme en C#. Volume (254 par défaut), cylindre et numéros restent sur un octet. La validation commune de face, secteurs présents et durées précède les contraintes Apple II ; le volume est vérifié avant le format, puis chaque taille avant son numéro.

L’ordre demandé est conservé. Chaque secteur comprend 100 cellules à un, le prologue d’adresse du format, les quatre valeurs 4-and-4, l’épilogue, trois octets FF, le prologue des données, leur bloc GCR, l’épilogue et 32 cellules à zéro. La révolution réutilise la construction commune. Marque supprimée, taille imposée et champs enregistrés ne sont pas exploités, conformément à cet encodeur C#.

## Décodage

Les intervalles passent par la reconstruction NRZI. Le décodage direct des bits utilise une cellule de durée 1. Une copie circulaire limitée aux 4 096 premiers bits disponibles permet de reconnaître les champs à cheval sur la fin de piste ; seules les positions d’adresse/prologue de la période d’origine déclenchent une recherche.

La passe 16 secteurs précède la passe 13 secteurs, puis les données non appariées sont relevées. Aucun tri final n’est ajouté. Les adresses sont lues en octets bruts, puis décodées 4-and-4. Les données suivantes sont recherchées dans une fenêtre de 1 024 bits à partir de la fin d’en-tête. Le prologue est marqué apparié même si ses données ne peuvent pas être décodées.

Les quatre octets d’adresse sont ajoutés au résultat uniquement dans la passe 16 secteurs. Les données tronquées en 16 secteurs produisent une structure tronquée ; la passe 13 secteurs n’en ajoute pas dans ce cas. Les longueurs d’adresse et avances après reconnaissance reprennent les distinctions source. Une adresse ou des données invalides rendent le secteur invalide ; sans données et sans invalidité connue, l’intégrité reste indéterminée. Les données à checksum faux restent accessibles. La face logique décodée est zéro, la taille 256 octets et le code 1. Tags, format, CRC enregistrés et intégrités séparées restent absents.

## Traduction et ressources

Les huit messages de MediaEngine/appleii-gcr-track.json portent deux erreurs et six recettes de descriptions. Les paramètres de coordonnées, volume et états d’intégrité restent structurés ; les 29 catalogues ont été générés avec Argos puis relus/corrigés, notamment pour les noms techniques, les nombres et la distinction données non appariées/tronquées. Les clés partagées d’état d’intégrité sont réutilisées.

Les buffers, secteurs, résultats et ressources du banc sont possédés et libérés par RAII, y compris à l’annulation et sur exception. Aucun accès matériel ni dépendance d’App n’est ajouté. Le catalogue de codecs par défaut complet et les politiques de reconstruction Apple restent à porter.
