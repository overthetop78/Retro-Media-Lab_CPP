# TYCOM FM — référence C#

Sources locales en lecture seule dans `F:/GW GUI/src/RetroMediaLab.MediaEngine`, relevées le 15 novembre 2026 : Images/Reading/Decoding/Definitions/TycomFmFormat.cs (avec TycomFmCrc et les modèles), Images/Reading/Decoding/Decoders/TycomFmDecoder.cs et Images/Writing/Encoding/Encoders/TycomFmTrackEncoder.cs.

Codecs/Tycom sépare les définitions (Constants.h), les lectures et recherches (Fields.cpp), l’émission (Encoder.cpp), la reconnaissance (Decoder.cpp) et leurs contrats (Fm.h). Les marques physiques sont celles de Codecs/FmAddressMarks ; le CRC est celui de Codecs/Crc16. La lecture FM double largeur bornée et annulable est ajoutée à Models/Flux/BitReader, sans modifier les lectures MFM existantes.

Les secteurs contiennent 128 octets. Le CRC CCITT initialisé à FFFF couvre FE/cylindre/secteur pour l’en-tête, et marque/charge utile pour les données ; il est émis fort-faible. Les octets sont codés en FM double largeur, avec gaps de 64 cellules à un. F8 représente les données supprimées, FB les données normales ; F9/FA sont aussi acceptés à la lecture.

Un en-tête illisible produit une structure tronquée. Un en-tête au CRC incorrect produit seulement sa structure d’erreur : aucun secteur et aucun octet d’adresse ne sont ajoutés, et ses données ne sont pas appariées. Les en-têtes valides ajoutent cylindre et secteur, puis les charges utiles décodées.

La recherche des données commence à la fin de l’en-tête et accepte une marque à distance maximale de 1 664 cellules, borne incluse. Elle s’arrête devant le prochain en-tête. Une marque appariée mais un bloc incomplet produit une structure tronquée. La face logique est zéro ; les métadonnées optionnelles d’intégrité enregistrée et de taille sont ignorées comme en C#.

La description d’un en-tête valide utilise la marque des données associées, y compris pour un bloc tronqué ; elle omet la marque lorsqu’aucune donnée n’est trouvée. La description d’un en-tête au CRC incorrect utilise FE et une taille nulle. Les structures des deux passes sont triées de manière stable par position. Les marques non appariées sont décrites comme non classifiées.

Les positions C++ utilisent des entiers 64 bits, avec protection des sommes/décalages et refus des comptes FM négatifs ou excessifs avant allocation. Une lecture FM vide retourne un tampon vide quel que soit son départ, conformément au contrat commun de lecture par blocs du port. Les lectures non vides hors source retournent une absence de résultat. Les 29 catalogues ont été générés avec Argos puis relus et harmonisés avec les termes techniques existants.
