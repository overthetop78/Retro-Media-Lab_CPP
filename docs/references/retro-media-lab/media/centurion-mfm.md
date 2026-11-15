# Centurion MFM — source de référence

Référence locale en lecture seule : `F:/GW GUI/src/RetroMediaLab.MediaEngine`, relevée le 15 novembre 2026.

- `Images/Reading/Decoding/Definitions/CenturionMfmFormat.cs` : identifiant, marques physiques, tailles, allocation, CRC et progression.
- `Images/Reading/Decoding/Definitions/CenturionMfmDescriptions.cs` : descriptions complètes, tronquées et non appariées.
- `Images/Writing/Encoding/Encoders/CenturionMfmTrackEncoder.cs` : construction privée des champs et émission des pistes.
- `Images/Reading/Decoding/Decoders/CenturionMfmDecoder.cs` : reconstruction adaptative MFM et reconnaissance.

## Organisation C++

`Codecs/Centurion/Constants.h` rassemble l’identité technique et les paramètres du format. `Fields.cpp` construit l’identité et les champs de données ; `Encoder.cpp` émet les pistes ; `Decoder.cpp` reconnaît les structures. `Mfm.h` expose les contrats internes du codec. Ces fichiers restent dans MediaEngine ; aucun accès matériel ou contrôle graphique n’y est ajouté.

## Comportement conservé

L’en-tête contient cylindre, secteur et CRC CCITT initialisé à zéro. Les données sont complétées de zéros par blocs de 256 octets, avec un minimum d’un bloc même pour une entrée vide et un maximum de 255 blocs. Le champ émis est préfixe réservé, nombre de blocs, zéro, charge utile complétée, CRC. Le CRC couvre le nombre de blocs, le zéro et la charge utile.

Le décodeur interprète le préfixe en clé puis longueur sur deux octets. Il conserve donc les longueurs déclarées non multiples de 256, les clés non prises en charge et les états d’intégrité indéterminés des champs incomplets. La face logique est zéro ; les métadonnées sectorielles optionnelles n’altèrent pas l’encodage. Une entrée de 257 octets produit 512 octets après complément.

La recherche de données commence 400 cellules après la fin de l’en-tête et s’arrête au prochain en-tête. Les quatre octets d’en-tête, CRC compris, sont ajoutés aux octets décodés, même avec un CRC incorrect. Les structures de données précèdent l’en-tête associé dans le résultat ; les marques non appariées sont ajoutées dans une seconde passe. Aucun tri supplémentaire n’est introduit.

Le calcul C++ du nombre de blocs évite l’addition susceptible de déborder dans le calcul C# `(data.Count + 255) / 256`. Les bornes de recherche utilisent des entiers 64 bits. Cette protection ne change aucun des cas de référence testés et n’a pas nécessité de fichier volumineux.

Les erreurs et descriptions sont résolues par clés de traduction ; 29 catalogues ont été générés avec Argos puis relus et harmonisés avec les termes déjà validés. Le catalogue de décodeurs complet et les politiques de reconstruction restent des blocs distincts.
