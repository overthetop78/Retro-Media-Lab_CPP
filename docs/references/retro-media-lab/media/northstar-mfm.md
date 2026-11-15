# Codec NorthStar MFM et secteurs matériels

Bloc 46 — portage de NorthstarMfmTrackEncoder, NorthstarMfmDecoder, NorthstarMfmFormat et RotatingChecksumCalculator, avec trente-cinq sources C# originales pour comparaison.

## Organisation et encodage

Codecs/Northstar sépare constantes, adresses, contrats, encodeur et décodeur. Codecs/RotatingChecksum est commun : XOR avec chaque octet, puis rotation d’un bit à gauche sur huit bits ; il correspond aussi au besoin Heathkit indiqué par la source C#. Il ne dépend d’aucun contrôle App.

L’adresse compacte emploie quatre bits de cylindre et quatre bits de numéro de secteur. Les primitives Pack/Unpack masquent les bits comme en C#, même pour des entrées signées hors plage ; l’encodeur impose en revanche les plages zéro à quinze. Après validation commune de la requête, chaque secteur est contrôlé dans l’ordre taille 512, cylindre, numéro. Les métadonnées sectorielles sont ignorées par cet encodeur.

La marque est constituée de sept octets nuls puis FB, encodés MFM, et écrite comme cellules brutes. Adresse, données et checksum sont écrits par trois appels MFM distincts, avec état initial indépendant. Un gap de 128 cellules nulles suit chaque secteur. L’ordre demandé est conservé.

## Décodeur de piste

Le flux est reconstruit en MFM adaptatif. Chaque marque de 128 cellules est examinée ; identité et bloc de données sont lus séparément. Une identité seule crée déjà un secteur de taille 512 avec données et intégrité absentes. Un bloc complet conserve les données même si sa somme est fausse ; les octets décodés concatènent alors identité compacte et données. Si l’identité manque, seule une structure tronquée reste.

La face logique vaut toujours zéro et le code de taille deux. La structure complète mesure 8 352 cellules ; avec identité seule, 144 ; sans identité, 128. Le balayage reprend 128 cellules après chaque marque reconnue et peut examiner une marque dans les données. Aucun suffixe circulaire ni tri ajouté. La confiance commune utilise poids sectoriel deux et diviseur vingt.

## Fenêtres de secteurs matériels

DecodeHardSectors est porté avec ses deux branches : reconstruction FM et synchronisation FM de deux octets pour des secteurs de 256 octets ; reconstruction MFM et synchronisation MFM de deux octets pour des secteurs de 512 octets. Le décalage de données est respectivement deux ou trois octets après la synchronisation. Les lecteurs de données source restent de type MFM dans les deux cas ; les bits d’horloge ne font pas l’objet d’une validation supplémentaire.

Le nombre de cellules est l’arrondi de la durée d’index divisée par la durée estimée d’une cellule, avec égalités arrondies vers l’entier pair comme Math.Round. Les fenêtres divisent ce nombre de cellules en sectorCount parties ; leur fin est bornée par le nombre réel de cellules reconstruites. Le candidat à checksum valide le plus tôt dans la fenêtre est retenu ; à défaut, le dernier candidat lisible, même invalide, est conservé. Une fenêtre sans candidat ne crée aucun secteur.

Les adresses cylindre/face sont celles fournies au helper, vérifiées sur un octet seulement lorsqu’un candidat existe. Les numéros sont ceux des fenêtres et les codes de taille sont un en FM, deux en MFM. Un nombre de secteurs nul ou négatif donne une liste vide après contrôle de la durée d’index. La source peut lever OverflowException pour l’arrondi ou les adresses ; le C++ utilise des diagnostics traduisibles correspondants. Le produit cellules × numéro de fenêtre est calculé sur 64 bits pour éviter le débordement int32 non contrôlé de la source pour des géométries extrêmes. Les recherches cessent une fois leurs départs au-delà des cellules disponibles ; elles conservent les résultats et évitent les itérations inutiles.

## Traductions et ressources

Sept textes sont générés par Argos, puis relus/corrigés dans les 29 langues. Les formulations de taille, plage et en-tête réutilisent les traductions déjà corrigées ; NorthStar est un invariant commun. Les réserves de buffers, résultats et registre sont possédées et libérées par RAII. Les points d’entrée et boucles vérifient l’annulation. Le décodeur s’insère dans le registre explicite, sans créer prématurément le catalogue complet.
