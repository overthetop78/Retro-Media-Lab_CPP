# Membrain MFM — référence C#

Sources locales en lecture seule, relevées le 15 novembre 2026 : `F:/GW GUI/src/RetroMediaLab.MediaEngine/Images/Reading/Decoding/Definitions/MembrainMfmFormat.cs`, `Decoders/MembrainMfmDecoder.cs` sous le même dossier Decoding et `Images/Writing/Encoding/Encoders/MembrainMfmTrackEncoder.cs`.

Le format porte aussi MembrainMfmAddress et les modèles MembrainMfmHeader/Data. Le port les répartit dans Codecs/Membrain : Constants.h pour les définitions, Address.cpp pour les adresses, Fields.cpp pour les lectures/recherches, Encoder.cpp pour l’émission, Decoder.cpp pour la reconnaissance et Mfm.h pour leurs contrats internes.

L’adresse utilise cinq bits hauts et trois bits bas de cylindre, une face et quatre bits de secteur. Les deux octets sont construits sans décalage signé indéfini en C++. Le CRC utilise le polynôme IBM 8005 et la valeur initiale zéro, avec émission fort-faible. Le secteur contient 512 octets. L’encodeur émet une synchronisation A1 spéciale suivie de FE pour l’en-tête et F8 pour les données, avec gaps de 64 et 128 cellules. Les métadonnées optionnelles et l’état supprimé sont ignorés comme en C#.

La lecture accepte F8 à FB après la synchronisation 4489. La recherche commence une cellule après l’en-tête et exige une marque complète dans la fenêtre de 832 cellules. Un en-tête intermédiaire ne l’arrête pas. Les helpers de champs vérifient leurs marques mais n’imposent pas une synchronisation A1 à l’octet initial ; la recherche physique la vérifie séparément.

Les six octets d’en-tête, CRC compris, sont conservés, même avec un CRC incorrect. Les données complètes précèdent leur en-tête dans les structures du résultat ; les marques non appariées sont ajoutées ensuite. Aucun tri supplémentaire n’est effectué. L’intégrité reste indéterminée lorsqu’aucune donnée n’est décodée et que le CRC d’en-tête est valide ; un CRC d’en-tête incorrect impose faux.

Les bornes de recherche utilisent des entiers 64 bits et une limite ramenée à la longueur de la source. Les limites extrêmes retournent une absence de marque sans débordement. Les diagnostics et descriptions sont traduits par clés, avec 29 catalogues générés par Argos puis relus et harmonisés avec les formulations déjà validées. Le catalogue complet et les politiques de reconstruction restent distincts.
