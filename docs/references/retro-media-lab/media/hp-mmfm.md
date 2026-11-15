# Pistes HP MMFM

Bloc 51 — portage de HpMmfmTrackEncoder, HpMmfmDecoder, HpMmfmFormat, HpMmfmDescriptions et HpMmfmCodec. Trente-sept fichiers C# originaux sont compilés sans modification pour comparaison.

## Organisation et transformation de charge utile

Codecs/HpMmfm sépare constantes, contrats/helpers, permutation, encodeur et décodeur. Le CRC CCITT 1021 hexadécimal avec initialisation FFFF réutilise Crc16 ; l’inversion des huit bits réutilise Primitives/Bits partagé.

EncodePayload échange les deux octets de chaque paire puis inverse leurs bits. DecodePayload inverse les bits puis échange les paires. Ces opérations commutent et chacune est involutive ; une même implémentation C++ produit donc les deux transformations, en écrivant reverse(payload[i+1]) puis reverse(payload[i]) dans un tampon possédé. Les deux points d’entrée restent distincts et sont comparés séparément aux sources originales. Une longueur impaire est refusée ; zéro et toute longueur paire sont admis par les helpers. Les pistes imposent pour leur part exactement 256 octets.

## Encodage de piste

Après les validations communes, l’encodeur contrôle cylindre sur un octet et face sur un bit. Il vérifie chaque taille de 256 octets avant le numéro de secteur de zéro à 127. Le secteur et la face sont empaquetés sur sept bits plus bit haut ; cet octet et le cylindre sont inversés bit à bit avant leur CRC.

La synchronisation d’en-tête est 55552A54 et celle des données 55552A44. Les champs protégés par CRC sont écrits avec la primitive MFM commune, conformément au C# : deux octets d’identité plus CRC, puis 256 octets de charge utile transformée plus CRC. Les synchronisations physiques sont écrites brutes. Les gaps contiennent 128 puis 256 cellules nulles. Les métadonnées de CRC/adresse, la taille forcée et Deleted restent ignorés comme dans cet encodeur ; l’ordre des secteurs est conservé.

## Lecture et association des champs

Le flux passe par la reconstruction MFM adaptative. L’en-tête doit être entièrement disponible (96 cellules) pour être examiné ; il n’existe pas de structure de troncature séparée. Le CRC porte sur les quatre octets lus après la synchronisation. Les deux premiers sont inversés ; la face est extraite du bit haut du secteur inversé. Le lecteur d’octets conserve le comportement source d’extraction des données sans validation supplémentaire des bits d’horloge.

Les données sont recherchées entre headerOffset + 160 et headerOffset + 928, positions de début de synchronisation incluses. La borne haute réelle impose également que les 32 cellules de synchronisation soient présentes. Une position est consommée dès sa première association à un en-tête, même si les données sont tronquées. Les en-têtes suivants ne relisent pas ce même bloc. Le CRC des données porte sur les 258 octets encodés ; seuls les 256 premiers sont ensuite inversés/permutés pour la charge utile logique.

Un CRC d’en-tête faux n’empêche pas la création d’un secteur ni la lecture des données. L’intégrité globale vaut faux si l’en-tête ou les données sont faux ; sinon, elle reste indéterminée sans données et vraie avec données valides. Les octets décodés contiennent les charges utiles complètes, même si leur CRC ou celui de l’en-tête est faux. Le secteur a code un, taille 256 et intégrité CRC. Tags, format, CRC enregistrés et intégrités séparées restent absents comme dans la source.

Les structures de données sont ajoutées avant leurs en-têtes et ne sont pas triées. Le format conserve donc leur ordre de production source, même si leurs positions décroissent à l’intérieur d’une paire. Le balayage avance de 32 cellules après un en-tête, sans sauter les données. Il n’existe pas de collecte des synchronisations de données non appariées ni de suffixe circulaire. La confiance commune utilise poids sectoriel deux et diviseur vingt.

Les helpers de lecture peuvent être appelés directement sur des cellules : ils n’exigent pas la synchronisation à l’offset fourni. Les positions négatives permettant un début de champ valide après les 32 cellules restent utilisables comme en C#. DecodeBytes réutilise la lecture MFM commune et ses diagnostics de compte. Les calculs de bornes utilisent des entiers 64 bits ; une recherche située entièrement hors des cellules retourne l’absence.

## Ressources et traductions

Résultats et tampons sont possédés par RAII ; les huit points d’entrée et les boucles contrôlent l’annulation. Les cinq textes sont générés par Argos puis relus/corrigés dans les 29 langues. Les descriptions CRC réutilisent les formulations ISO FM déjà corrigées. HP et MMFM sont des invariants communs.
