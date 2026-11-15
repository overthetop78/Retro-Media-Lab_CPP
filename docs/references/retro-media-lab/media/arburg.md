# Blocs Arburg — référence C#

Sources locales en lecture seule dans `F:/GW GUI/src/RetroMediaLab.MediaEngine`, relevées le 15 novembre 2026 :

- `Images/Reading/Decoding/Definitions/ArburgFormat.cs` : deux variantes, tailles, marques, attribut et descriptions.
- `Images/Reading/Decoding/Definitions/ArburgChecksum.cs` : somme additive 16 bits, ordre faible/fort et remplissage.
- `Images/Reading/Decoding/Definitions/ArburgSystemCodec.cs` : codage variable système et position de lecture.
- `Images/Writing/Encoding/Encoders/ArburgTrackEncoder.cs` : sélection et émission des blocs.
- `Images/Reading/Decoding/Decoders/ArburgDecoder.cs` : deux balayages et fusion de leurs résultats.

## Organisation C++

`Codecs/Arburg/Checksum.cpp` rassemble la somme et la construction des blocs ; `System.cpp` isole la transformation variable. `Encoder.cpp` émet les deux variantes et `Decoder.cpp` les reconnaît. `Constants.h` rassemble tailles, marques, paramètres et clés ; `Blocks.h` expose les contrats internes. Le codec reste dans MediaEngine et réutilise l’inversion d’octet et le double FM communs.

## Comportements conservés

Le bloc de données contient 2 558 octets utiles, puis deux octets de checksum, soit 2 560 octets physiques. Le bloc système contient 3 838 octets utiles pour 3 840 octets physiques. Le checksum est la somme des octets utiles réduite à 16 bits, enregistrée poids faible puis fort.

L’attribut **sectoriel** system sélectionne la variante ; toute valeur non nulle active le système. L’attribut de piste n’est pas utilisé comme repli. Les numéros, la marque supprimée et les métadonnées optionnelles des secteurs ne modifient pas le codage. Une entrée peut contenir seulement les octets utiles ou le bloc complet. Dans les deux cas, le checksum est recalculé depuis les octets utiles, sans conserver celui d’un bloc fourni.

Les données ordinaires sont inversées bit à bit puis émises en double FM. Le système émet chaque octet du bit faible au bit fort : zéro devient 01 et un devient 001. Un gap de 64 cellules à un sépare les blocs.

Le décodeur utilise la reconstruction MFM adaptative, puis deux balayages indépendants. Chaque reconnaissance avance seulement après la marque, sans sauter l’intégralité du bloc. Des marques contenues dans les cellules d’un bloc restent donc examinées. Les secteurs ont l’adresse logique C0/H0/R1 et le code de taille zéro ; leur taille déclarée est physique, mais les données retournées n’incluent pas le checksum. L’intégrité utilise Checksum.

Les structures des deux balayages sont fusionnées puis triées de manière stable par position. Les secteurs et les octets décodés restent regroupés : données ordinaires, puis système. Une marque complète avec bloc absent ou incomplet reste visible avec intégrité indéterminée.

## Bornes et traductions

Les helpers C++ évitent les lectures ou écritures hors limites : capacité de bloc insuffisante/négative/non prise en charge et taille utile négative produisent des diagnostics nommés. Un compte système négatif ou supérieur à la limite est refusé ; pour une lecture non vide, un départ négatif ou hors de la source retourne une absence de résultat. Une lecture vide conserve son départ, comme le C#. Ces comportements remplacent les exceptions incidentes d’indexation/allocation du C# aux arguments invalides ; les cas valides sont inchangés.

Les positions utilisent des entiers 64 bits. Les huit points d’entrée possédant un jeton d’annulation le contrôlent. Les catalogues ont été générés avec Argos puis relus et corrigés dans les 29 langues, notamment pour préserver les invariants, les noms de structures et le sens « non négatif ».

Le catalogue complet des décodeurs et les politiques de reconstruction restent à porter séparément.
