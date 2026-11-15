# Codec de piste Apple RWTS18

Bloc 39 — portage de AppleRwts18TrackEncoder et AppleRwts18Decoder, avec les paramètres restants de AppleRwts18Format. Les transformations des trois pages restent isolées dans Payload.cpp.

## Encodage

Les validations communes de piste passent en premier. Le cylindre RWTS18 doit ensuite tenir sur six bits. La piste doit contenir exactement six secteurs uniques numérotés de 0 à 5 ; la taille de chacun est vérifiée dans l’ordre de la requête avant l’identifiant. Les secteurs sont émis de 5 à 0, indépendamment de l’ordre reçu. Une taille incorrecte conserve le numéro du secteur dans le diagnostic.

Chaque champ comprend le gap à un (200 bits avant le secteur 5, quatre avant les autres), la marque D59D, les symboles 6-and-2 de piste/secteur/XOR, la fin AA, deux FF, l’identifiant modifiable (A4 par défaut), les 1 025 symboles de charge utile et la fin D4 FF. Les identifiants sur tout l’octet sont autorisés comme en C#, même lorsque leur bit haut nul modifie la synchronisation à la lecture. Marque supprimée, code de taille forcé et métadonnées enregistrées ne sont pas interprétés par cet encodeur.

## Décodage

Le flux est reconstruit en NRZI ou reçu directement sous forme de bits. Le rebouclage copie au plus 16 384 bits disponibles ; une marque d’adresse doit tenir dans la période d’origine pour être considérée. Quatre octets synchronisés suivent D59D. Piste, secteur et XOR doivent appartenir à la table 6-and-2 ; le secteur doit être inférieur à six, le XOR correct et la fin égale à AA. Une adresse invalide n’ajoute ni structure ni secteur.

Une adresse valide déclenche la lecture synchronisée d’une fenêtre entière de 1 100 octets. Si cette lecture échoue, aucune donnée n’est retournée, même si un enregistrement plus court aurait tenu. Le premier identifiant suivi de 1 025 symboles GCR et de D4 est retenu ; l’identifiant lui-même n’a pas de valeur imposée. Un checksum faux conserve les données et leur intégrité fausse. Sans données, le secteur reste d’intégrité indéterminée.

Les positions de données suivent le calcul source offset + huit fois la position dans la fenêtre d’octets. Elles ne sont pas remplacées par les positions physiques réelles parcourues par la lecture synchronisée en présence de bits décalés. Le secteur contient 768 octets, code de taille 3 et face logique zéro. Tags, format et CRC enregistrés restent absents. L’ordre des structures est adresse puis données. La confiance combine secteurs valides/6 et secteurs détectés/24, plafonnée à un.

## Organisation et ressources

Codecs/AppleRwts18 comprend les contrats Gcr.h, constantes, Payload.cpp, Encoder.cpp et Decoder.cpp. La lecture AppleBitLatch et la table Apple II sont partagées. Le décodeur peut être fourni au registre explicite ; aucun catalogue par défaut réduit n’est ajouté.

Les erreurs de disposition et de plage ainsi que les deux descriptions structurées sont dans MediaEngine/rwts18-track.json et ses 29 catalogues Argos relus. Les traductions de coordonnées, sommes de contrôle et limites reprennent les formulations déjà corrigées du codec Apple II. Buffers, secteurs et résultats sont possédés ; RAII et contrôles d’annulation couvrent aussi les sorties par exception.

Les classes de codec et AppleRwts18Format sont portées intégralement. Les politiques Apple de reconnaissance/reconstruction, la conversion en image sectorielle et les services de documents restent des composants distincts non annoncés comme portés par ce bloc.
