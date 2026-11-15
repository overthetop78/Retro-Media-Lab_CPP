# Codec E-mu FM

Bloc 44 — portage de EmuFmTrackEncoder, EmuFmDecoder, EmuFmFormat et EmuFmDescriptions (identité incluse), avec le CRC et les fonctions FM communes. Trente-six sources C# originales sont compilées pour la comparaison.

## Encodage

Codecs/Emu sépare constantes, contrats, encodeur et décodeur. Le cylindre doit tenir sur sept bits et la face sur un bit, après validation commune. Leur combinaison cylindre décalé/face est inversée bit à bit pour former la piste enregistrée. Chaque charge utile contient exactement 3 584 octets. Le numéro fourni au secteur est ignoré, même hors plage d’un octet, comme dans le C# ; le numéro logique décodé est toujours un.

En-tête et données partagent la marque 4545555545545445. L’en-tête contient piste brute puis CRC fort/faible. Les données comprennent la charge utile puis CRC fort/faible. Les octets sont encodés avec DoubleFm, soit 32 cellules par octet ; chaque champ est suivi de 64 cellules à un. Les CRC sont IBM non réfléchis, polynôme 8005 hexadécimal et initialisation nulle, calculés par le service commun. Les métadonnées de CRC enregistrés, marques supprimées et tailles forcées ne modifient pas cet encodeur, conformément à la source.

## Décodage et classification

Le flux passe par la reconstruction MFM adaptative commune, comme en C#, puis les octets sont lus par leur quatrième bit de chaque groupe de quatre cellules. Les autres bits d’horloge ne sont pas validés séparément. Un en-tête doit être entièrement lisible et produire un CRC nul ; sinon sa marque reste à classifier. La piste lue est inversée à nouveau puis séparée en cylindre et face.

Après un en-tête valide, la prochaine marque est recherchée depuis sa fin, à distance maximale 1 664 bits. La borne de départ maximale est inclusive. Un bloc de données complet doit fournir 3 586 octets, dont deux de CRC. Un CRC faux conserve les données et l’intégrité fausse. Les données tronquées restent absentes, l’intégrité du secteur est indéterminée et leur marque n’est pas classée comme données.

Les marques d’en-tête et de données sont identiques. Le balayage d’en-têtes ne saute que 64 bits après une reconnaissance ; les premiers octets d’une charge utile peuvent donc former un nouvel en-tête si leur CRC est nul. Ce comportement source est conservé et testé. Les marques classées n’empêchent pas leur examen par cette première passe ; elles servent à la collecte finale des marques non classées. Les structures sont triées de façon stable par position.

La suite décodée conserve une adresse recomposée non inversée par en-tête valide, puis les données si présentes. Les secteurs utilisent l’intégrité CRC, code de taille zéro et taille 3 584. Tag, format, CRC enregistrés et intégrités séparées restent absents. Aucun suffixe circulaire n’est ajouté.

## Traductions et ressources

Les deux erreurs et trois descriptions de MediaEngine/emu-fm.json sont générées avec Argos puis relues dans les 29 langues. Les formulations d’adresse et de taille déjà corrigées sont réutilisées. E-mu et E-mu Emulator sont protégés dans la liste commune d’invariants ; le nom de modèle Emulator n’est pas traduit.

Les buffers, résultats et objets de registre sont possédés et libérés par RAII. Les points d’entrée et boucles contrôlent l’annulation. Les données classées utilisent un ensemble de positions. Le codec peut être fourni au registre explicite ; catalogue complet et politiques de reconstruction restent distincts.
