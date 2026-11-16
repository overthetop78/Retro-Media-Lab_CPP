# Lecteur 86F

Les neuf sources de `F:/GW GUI/src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/I86f/` ont été consultées en lecture seule. Leurs empreintes SHA256 actuelles concordent avec `inventaire-csharp.csv` : lecteur, adaptateur protégé, format, disposition, exceptions, deux modèles et deux familles de drapeaux.

## Organisation

`Constants.h` porte les champs et masques binaires, `I86f.h` les modèles et contrats, `Reader.cpp` le décodage borné, `ProtectedImage.cpp` la conversion vers les pistes protégées communes et `Adapter.cpp` l'intégration au registre. Les valeurs brutes des drapeaux de fichier et de piste sont conservées, y compris leurs bits non interprétés.

## Lecture fidèle au C#

La signature little-endian est 0x46423638. Le champ de version n'est pas interprété par le lecteur C# et ne reçoit pas de restriction supplémentaire. La table contient 256 entrées par face ; la seconde face occupe les entrées 256 à 511, sans alternance des faces. Une entrée nulle est absente. La prochaine entrée non nulle dans l'ordre logique borne la piste.

L'en-tête de piste mesure six ou dix octets suivant le drapeau du champ supplémentaire. Le nombre explicite signé de cellules est utilisé seulement si ce champ est présent, si le masque de vitesse est nul et si le drapeau de nombre explicite est actif. Sinon, le nombre de cellules est déduit de la plage. Le stockage est arrondi au mot de 16 bits ; les deux octets du mot peuvent être inversés, mais les bits de chaque octet restent lus du poids fort au poids faible. Une piste ne contenant aucune transition est omise.

Les contrôles reprennent ceux du C# : table complète, offsets représentables, en-tête accessible, prochain offset après l'en-tête, nombre de cellules positif et données alignées disponibles dans le fichier. Le nombre explicite peut décrire des données au-delà du prochain offset tant qu'elles existent dans le fichier : le port ne remplace pas cette règle par une nouvelle validation plus restrictive.

## Conversion protégée

La cadence reste celle du C# : 40 ticks de 25 ns par cellule, soit 1000 ns. La durée d'index correspond au nombre de bits multiplié par 40, vérifié avant conversion en entier non signé. La fabrique de révolutions existante conserve aussi l'intervalle final.

Les coordonnées proviennent de l'index logique modulo/divisé par 256. Chaque piste possède ses bits, un segment de cadence et une révolution. Les listes de structures et de caractéristiques restent vides comme dans le C# ; ce lecteur ne déduit pas de zones faibles ni une nouvelle cadence à partir des drapeaux de vitesse. La représentation n'est pas marquée protégée en écriture.

Le registre expose `raw.86f`, `.86f`, la représentation flux et la métadonnée décimale `86f.flags`. Les erreurs structurelles et l'absence de pistes utilisables sont des diagnostics traduits et des rejets contrôlés du registre. L'annulation est vérifiée dans le parcours et les conversions. Aucun writer 86F n'est ajouté : ces sources C# n'en contiennent pas.
