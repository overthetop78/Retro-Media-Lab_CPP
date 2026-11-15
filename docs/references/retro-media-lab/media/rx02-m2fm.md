# Primitives RX02 M²FM et marques FM

Bloc 49 — portage de DecRx02M2FmCodec, des définitions de marques et sélections de DecRx02Format et de FmAddressMarkPatterns. Quinze fichiers C# originaux sont compilés pour la comparaison. Le décodeur/encodeur complet de piste RX02 et ses descriptions restent le bloc suivant ; les formats de fichier RX02 et les politiques de reconstruction ne sont pas encore portés.

## Marques physiques partagées

Codecs/FmAddressMarks fournit les sept motifs fermés des marques F8 à FE, chacun sur quatre octets de cellules physiques. Les motifs sont ceux du C# (55111444, 55111445, 55111454, 55111455, 55111544, 55111545, 55111554) ; toute autre valeur est rejetée avec un diagnostic traduisible.

Codecs/DecRx02 définit les six marques de données dans le même ordre source : F8 FM supprimée, F9 M²FM normale, FA FM normale alternative, FB FM normale, FC FM normale alternative, FD M²FM supprimée. Leurs tailles sont 128 en FM et 256 en M²FM, codes zéro/un. La marque d’en-tête est FE. EncodingForSize accepte uniquement les tailles 128 et 256. DataMarkFor sélectionne les quatre marques canoniques F8/F9/FB/FD ; conformément à la comparaison source, toute valeur d’énumération différente de M2Fm suit la branche FM.

## Transformation M²FM

La transformation reçoit des cellules MFM déjà encodées. Le motif normal de onze cellules 00101010100 devient 01000100010. L’encodage cherche depuis la cellule un, avance par deux et saute huit cellules supplémentaires après un remplacement. Le remplacement modifie la séquence en place, sans suffixe circulaire.

Pour décoder count octets, le helper demande count × seize cellules depuis start, puis les copie dans un tampon précédé d’une cellule nulle. Le remplacement inverse commence à zéro, avance d’une cellule, exige la même parité que zéro et saute neuf cellules supplémentaires après un remplacement. L’extraction ignore la cellule initiale et examine ensuite chaque paire d’horloge/donnée : 11 rejette le bloc, 01 vaut un, 00 et 10 valent zéro. Un bloc tronqué retourne l’absence. Un compte nul retourne une suite vide si le départ n’est pas au-delà de la source, y compris pour un départ négatif comme en C#.

## Fiabilité et différences de bornes

Le C++ contrôle le compte avant toute multiplication/allocation, utilise des bornes sur 64 bits et refuse les cellules autres que zéro/un. Un départ négatif pour une lecture non vide est un diagnostic explicite. Le compte maximal est limité par la capacité des flux commune (2 147 483 647 cellules, tampon de phase inclus).

Ces gardes renforcent deux comportements aux coordonnées invalides du C#. Un compte négatif ou un produit débordé pouvait parfois retourner l’absence avant l’allocation suivant start ; le C++ refuse systématiquement ces comptes. Un départ positif extrême pouvait déborder dans start + requiredBits, contourner la vérification de longueur et provoquer IndexOutOfRangeException ; le C++ retourne proprement l’absence de bloc hors limites. Le banc vérifie et compte séparément ces différences attendues, sans les confondre avec une équivalence des entrées invalides. Les entrées usuelles et tous les motifs comparés conservent le comportement source.

Les tampons temporaires et résultats sont possédés par RAII. Les points d’entrée et boucles contrôlent l’annulation. L’encodeur est une opération en place ; un appelant doit écarter son tampon de travail si l’opération est interrompue en cours de transformation.

## Traductions

Les trois erreurs propres au bloc sont générées avec Argos puis relues/corrigées dans les 29 langues. Les gardes génériques de taille de flux, compte d’octets et valeur binaire réutilisent les diagnostics traduits existants. DEC, RX01, RX02 et M²FM sont des invariants communs.
