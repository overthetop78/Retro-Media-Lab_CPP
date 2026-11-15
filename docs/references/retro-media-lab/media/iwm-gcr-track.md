# Pistes Apple IWM Macintosh et Lisa

Bloc 41 — portage de AppleIwmGcrDecoder et des identités Macintosh/Lisa, de AppleMacGcrTrackEncoder et de sa spécialisation Lisa, avec les paramètres restants d’AppleIwmGcrFormat. Quarante sources C# originales sont compilées pour la comparaison.

## Organisation et identités

Codecs/AppleIwm conserve les primitives de charge utile et ajoute Gcr.h, Encoder.cpp et Decoder.cpp. Une variante explicite sélectionne applemac.gcr / Apple Macintosh GCR ou applelisa.fileware.gcr / Apple Lisa FileWare GCR. Les algorithmes sont communs et le format par défaut vaut 12 hexadécimal dans les deux cas. Les descriptions techniques restent nommées Apple Macintosh, même pour l’identité Lisa, comme dans la source ; ce libellé n’est pas déduit du nom de codec.

## Encodage

Après les validations communes de piste, cylindre, face et format sont contrôlés ; le format tient sur six bits. Chaque secteur conserve l’ordre demandé. Sa taille doit être de 512 octets, son numéro sur six bits. Les champs d’adresse regroupent cylindre bas, secteur, bits hauts du cylindre et face, format, puis XOR sur six bits. Ils sont convertis par la table Apple II partagée et entourés des marques/gaps source.

Les douze attributs tag0 à tag11 sont lus séparément, nuls par défaut et validés sur un octet. Ils précèdent les données pour la transformation IWM de 524 octets. Le champ de données comprend son prologue, le symbole de numéro, les 703 symboles de charge utile/checksum et l’épilogue. Les tailles imposées, marques supprimées et métadonnées enregistrées sont ignorées comme par cet encodeur C#. Les gaps sont des cellules à un.

## Décodage

Le codec accepte la reconstruction NRZI automatique, une durée imposée sans adaptation, ou les bits directs. Un suffixe circulaire de 8 192 bits au plus est ajouté. Une marque d’adresse dans la période d’origine déclenche la lecture brute de cinq symboles. Un en-tête présent avec symbole invalide conserve sa longueur, tandis qu’une lecture tronquée garde seulement la longueur du prologue ; l’adresse peut être indécodable dans les deux cas.

Une adresse dont les valeurs GCR sont lisibles conserve coordonnées, format et état XOR même si le checksum est faux. Seule une adresse valide permet la recherche du prologue de données dans une fenêtre de 512 bits. Le prologue trouvé est marqué apparié avant lecture des 704 symboles. Les symboles inconnus ou manquants produisent une structure tronquée ; les données non appariées restent des structures distinctes.

Le numéro présent dans les données est lu mais n’est pas comparé à celui de l’adresse, conformément à la source. Les 699 valeurs de charge utile sont décodées, puis les quatre valeurs de checksum enregistrées comparées aux valeurs reconstruites. Douze octets de tags sont conservés séparément ; seuls les 512 octets de données sont ajoutés à la suite décodée. Une intégrité fausse domine, des données absentes restent indéterminées si l’adresse n’est pas connue invalide. Le format est conservé dès qu’une adresse est décodable. CRC enregistrés, marques supprimées et intégrités sectorielles séparées restent absents.

Les structures sont triées de façon stable par position finale, contrairement au codec Apple II antérieur ; les secteurs conservent leur ordre de découverte. L’avance après reconnaissance et la longueur des champs suivent le C#.

## Diagnostics et ressources

Les deux nouvelles erreurs et quatre descriptions de MediaEngine/iwm-gcr-track.json sont générées avec Argos puis relues dans les 29 langues. Les formulations Apple déjà corrigées sont réutilisées avec les bons paramètres, tailles et noms. Les clés d’état d’intégrité restent partagées.

Les résultats, tags, buffers, révolutions et objets de registre sont possédés et libérés par RAII. Les chemins directs, NRZI, durée imposée et encodage contrôlent l’annulation. Aucun accès matériel ni logique d’App n’est ajouté. Les codecs peuvent être enregistrés explicitement ; le catalogue complet et les politiques Apple de reconstruction restent à porter.
