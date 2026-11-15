# Charge utile Apple RWTS18

Bloc 38 — référence C# : AppleRwts18Codec, avec les constantes utiles de AppleRwts18Format. Neuf fichiers originaux sont compilés pour la comparaison ; leurs empreintes correspondent à l’inventaire.

Un secteur physique contient trois pages consécutives de 256 octets. Chaque position des pages produit quatre valeurs : un symbole regroupant les deux bits hauts de chaque page, puis les six bits bas des trois octets. Les 1 024 valeurs sont converties par la table 6-and-2 Apple II partagée ; leur XOR fournit le symbole final de checksum. L’encodage exige exactement 768 octets et signale le numéro conventionnel -1 pour une charge utile isolée incorrecte.

Le décodeur reçoit les valeurs numériques déjà extraites de la table, pas les symboles GCR enregistrés. Il reconstruit les trois pages, puis applique le calcul d’intégrité source avec accumulateur et octet précédent de la première page. Une somme incorrecte conserve les données et renvoie false. Le C# ne restreint pas ces valeurs à six bits : le C++ conserve les mêmes opérations sur huit bits et ne masque pas artificiellement les valeurs brutes. Les valeurs supplémentaires après le checksum sont ignorées, comme dans la source.

Une longueur inférieure à 1 025 valeurs provoque en C# une ArgumentOutOfRangeException(index) lors du premier accès manquant. Le C++ valide cette précondition avant accès et produit Rwts18PayloadLength avec taille reçue et minimum attendu ; aucune lecture hors limites n’est effectuée. Le comparateur associe ce diagnostic au refus source correspondant. Les autres tailles, erreurs et résultats sont conservés.

Codecs/AppleRwts18 sépare Codec.h (contrats possédés), Constants.h (tailles, masques, décalages et positions) et Payload.cpp (transformations). Buffers et résultats sont libérés par RAII ; les deux opérations contrôlent l’annulation à l’entrée et dans leurs boucles. Les deux diagnostics de MediaEngine/rwts18-payload.json et les 29 catalogues ont été générés avec Argos puis relus/corrigés. RWTS18 rejoint la liste commune d’invariants.

AppleRwts18Codec est porté intégralement. AppleRwts18Format reste partiel : les champs de piste, son ordre physique et les codecs Encoder/Decoder seront repris dans le bloc suivant. Aucune prise en charge de piste RWTS18 n’est encore annoncée au registre.
