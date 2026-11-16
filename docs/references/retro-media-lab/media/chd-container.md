# Conteneur commun CHD

Référence en lecture seule : les quatre sources ChdContainer.cs, ChdContainer.Map.cs, ChdContainer.Codecs.cs et ChdContainer.EccTables.cs ; empreintes vérifiées dans l’inventaire. Le découpage C++ sépare constantes, contrat/source, lecture d’en-tête et de hunks, cartes, composition des codecs et tables ECC.

La source RandomAccessData est possédée par shared_ptr ; un mutex sérialise la lecture des métadonnées et hunks avec leurs espaces de travail. Les codecs natifs et les buffers restent sous RAII même si la construction échoue. L’annulation est transmise aux sources, aux codecs, à l’analyse des cartes et au réassemblage CD.

Les en-têtes v1-v5 conservent les champs du C#, ses limites de 10 GiB logiques et 128 MiB par hunk, ses codecs historiques et son refus des parents. Les produits de dimensions sont contrôlés avant débordement. Les cartes historiques 8/16 octets, leur cookie terminal, les mini-patterns et références internes sont repris. Les cartes v5 non compressées utilisent des offsets multiples du hunk, avec zéro implicite pour une entrée nulle. Les cartes compressées utilisent Huffman, les deux répétitions RLE, références self/parent et CRC16 CCITT initialisé à FFFF. Les références parent restent refusées à la lecture ; les références cycliques sont bornées par le nombre de hunks. Les bornes des cartes historiques et cartes v5 brutes sont vérifiées avant allocation.

Les métadonnées sont liées par des offsets pouvant revenir en arrière. Le nombre de liens parcourus reste borné par la longueur physique ; les cycles échouent. Les textes sont ASCII, arrêtés au premier zéro, comme dans la référence.

La composition accepte zlib, LZMA, Huffman, FLAC et Zstandard, ainsi que cdzl/cdlz/cdfl/cdzs. Elle valide les codecs annoncés et les alignements ; FLAC porte un marqueur L/B pour l’ordre des octets. Les données principales CD et sous-codes sont décompressés séparément puis entrelacés en secteurs de 2448 octets (2352 + 96). Le bitmap ECC rétablit la synchronisation et les parités P/Q avec les quatre tables exactes du C#. Le mode 2 neutralise les quatre octets d’adresse lors du calcul ECC.

Cette couche ne remplace pas les lecteurs/writers HDD et optiques ni la composition globale des services : leur raccordement reste à porter.
