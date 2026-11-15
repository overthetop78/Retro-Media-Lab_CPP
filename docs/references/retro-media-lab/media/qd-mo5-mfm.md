# QD MO5 MFM — référence C#

Sources locales en lecture seule dans `F:/GW GUI/src/RetroMediaLab.MediaEngine`, relevées le 15 novembre 2026 : Images/Reading/Decoding/Definitions/QdMo5MfmFormat.cs (avec QdMo5Checksum et les modèles), Images/Reading/Decoding/Decoders/QdMo5MfmDecoder.cs et Images/Writing/Encoding/Encoders/QdMo5MfmTrackEncoder.cs.

Codecs/QdMo5 sépare les définitions (Constants.h), les champs/checksum/recherches (Fields.cpp), l’émission (Encoder.cpp), la reconnaissance (Decoder.cpp) et les contrats internes (Mfm.h). Les sources sont rangées dans leur groupe CMake ; aucun accès matériel ou graphique n’est introduit.

Le numéro de secteur est grand-boutiste sur 16 bits. Le préambule contient dix octets A914 répétés cinq fois ; la marque physique d’en-tête ajoute 4491. Treize octets réservés suivent l’adresse. L’encodeur les remplit de zéro ; le helper de lecture les restitue sans exiger cette valeur.

La charge utile contient 128 octets, précédés d’un préfixe sectoriel et suivis d’un checksum additif sur un octet (préfixe inclus). Il n’y a pas de CRC d’en-tête. L’attribut de piste n’impose pas un préfixe aux secteurs ; leur défaut est 5A. L’encodeur autorise les préfixes 00–FF, y compris FB ; à la lecture, FB est traité comme un nouvel en-tête, comportement C# conservé et comparé.

La recherche commence à la fin de l’en-tête et accepte une marque au plus à 832 cellules de ce départ, borne incluse. Elle s’arrête au prochain en-tête. Les positions sont calculées sur 64 bits, sans débordement de somme départ/distance. Les helpers de champs effectuent leurs lectures à un décalage fixe et ne revalident pas les motifs physiques, comme le C#.

Le résultat conserve les deux octets de numéro de secteur et les charges utiles complètes. Les secteurs ont C0/H0, taille 128, code zéro et intégrité Checksum. Les blocs incomplets et marques non appariées restent décrits ; les structures sont triées de manière stable par position après les deux passes. Les gaps sont de 160 et 128 cellules. Les métadonnées sectorielles optionnelles restent ignorées.

Les 29 catalogues ont été générés avec Argos puis relus et harmonisés avec les termes techniques existants. Les diagnostics de taille et de plages restent séparés de la présentation. Les mots « CRC » présents dans le commentaire C# de l’encodeur ne correspondent pas à l’algorithme réel ; le port suit le code et utilise le checksum additif.
