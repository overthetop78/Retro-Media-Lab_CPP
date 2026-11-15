# AED 6200P MFM — référence C#

Sources locales en lecture seule dans `F:/GW GUI/src/RetroMediaLab.MediaEngine`, relevées le 15 novembre 2026 :

- `Images/Reading/Decoding/Definitions/Aed6200pMfmFormat.cs` : marques et disposition physique.
- `Images/Writing/Encoding/Encoders/Aed6200pMfmTrackEncoder.cs` : validation, identité et émission CRC.
- `Images/Reading/Decoding/Decoders/Aed6200pMfmDecoder.cs` : appairage, progression, secteurs et descriptions.

## Découpage

`Codecs/Aed6200p/Constants.h` contient les marques, dimensions, limites et clés ; `Fields.cpp` construit l’identité ; `Encoder.cpp` émet les pistes ; `Decoder.cpp` reconstruit les structures. `Mfm.h` porte les contrats du codec. L’ensemble reste dans MediaEngine et réutilise les primitives partagées MFM et CRC. Aucun accès matériel ni code graphique n’y est ajouté.

## Règles conservées

L’identité est C6, cylindre, taille faible, numéro de secteur, taille forte. Le CRC CCITT utilise sa valeur initiale par défaut FFFF et s’écrit poids fort puis faible. Le motif physique d’en-tête 5094 remplace l’octet C6 ; les six octets suivants sont émis en MFM, puis un gap de 64 cellules.

Les quatre marques C0/C1/C2/C3 ont respectivement les motifs physiques 508A/5089/5084/5085. L’encodeur sélectionne C0 pour les données supprimées et C3 pour les données normales. Le CRC porte sur la marque et la charge utile. Le gap terminal contient 128 cellules. Les tailles de zéro à 65 535 sont acceptées à l’encodage ; les métadonnées sectorielles optionnelles n’altèrent pas le flux.

La recherche des données commence une cellule après le début de l’en-tête et exige que la marque complète soit dans la fenêtre de 104 octets bruts (832 cellules). Elle ne s’arrête pas à un en-tête intermédiaire. Une marque reconnue est enregistrée comme appariée avant la lecture du bloc.

Le décodeur conserve les sept octets d’en-tête, CRC compris, puis ajoute la charge utile lorsque la lecture est complète. Les tailles nulles et blocs incomplets produisent une structure de données tronquées et une intégrité indéterminée, sauf si l’en-tête est invalide. Les secteurs ont la face logique zéro ; l’état supprimé n’est pas ajouté aux métadonnées car le C# ne le fournit pas. Les descriptions de données utilisent C0 H0 R0 comme le C# ; les secteurs et descriptions d’en-tête conservent leur adresse réelle.

Les données sont ajoutées aux structures avant l’en-tête associé, puis les marques non appariées sont relevées dans une seconde passe. Aucun tri supplémentaire n’est introduit.

Les calculs de positions C++ utilisent des entiers 64 bits pour éviter les débordements lors des recherches. Les longueurs de charge utile sont vérifiées avant leur conversion en champ 16 bits. Les catalogues ont été générés avec Argos puis relus et harmonisés avec les formulations techniques déjà validées dans les 29 langues.

Le catalogue complet des décodeurs et les politiques de reconstruction restent à porter séparément.
