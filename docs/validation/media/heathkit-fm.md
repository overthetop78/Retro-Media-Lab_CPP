# Validation du codec Heathkit FM — 15 novembre 2026

Le banc compile trente-sept sources C# originales inchangées. Leurs SHA-256 correspondent à l’inventaire. L’empreinte du manifeste est `96e90e527acc87dc9bb2d51e3956e269c05f3f97da031ca5081b9ce3df3dd6b4`.

Les 1 297 cas comprennent 521 lectures de records, 265 encodages de records, 256 inversions d’octet, 123 décodages de piste, 88 encodages et 44 lectures directes de données. Les inversions sont comparées à BitPrimitives.ReverseBits original et aux deux namespaces C++ (primitif partagé et alias E-mu).

Les résultats comparent toutes les cellules et tous les intervalles uint32 little-endian, index, durée de cellule, confiance, octets, secteurs/métadonnées et structures : type, position, longueur, description et ordre. Les cas couvrent attribut de volume absent/vide/incorrect, casse et attribut sectoriel ignoré ; coordonnées, taille, durées et ordre de validation ; secteurs multiples et métadonnées ignorées ; checksums corrects/faux d’en-tête/données, intégrité indéterminée et données tronquées. Les limites de recherche testent 831, 832 et 833 cellules après la fin de l’en-tête, l’offset de marque étant inclus jusqu’à 832. Une marque appariée reste exclue comme en-tête même si ses données sont tronquées. Les marques imbriquées dans une charge utile sont aussi comparées.

La référence produit 1 240 résultats et 57 refus : 51 ArgumentOutOfRangeException, cinq ArgumentException et une IndexOutOfRangeException. Chaque refus est associé au diagnostic C++ correspondant, et non seulement à une exception quelconque : volume, numéro, limites communes, tailles, durées, absence de secteurs et record vide. Les tests avec plusieurs valeurs invalides vérifient ainsi la priorité des validations. Le cas du record vide remplace l’accès invalide source par un diagnostic de domaine explicite et traduisible.

Les décodages de piste comprennent 115 secteurs : 39 checksums globalement valides, 57 faux et dix-neuf indéterminés ; 129 structures d’en-tête et 76 de données. Les lectures directes produisent deux blocs valides, trente faux et douze absences, avec des offsets négatifs/limites et des positions de bits différentes.

Cinq contrôles supplémentaires vérifient l’annulation des points d’entrée ; un vérifie le registre automatique explicite avec volume/cylindre/secteur 255 et face demandée un, face logique zéro. Les 203 textes chargés depuis les DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64 et Qt 6.12.
- Chaque configuration : **1 506 contrôles, 0 différence, 1 297 cas, 203 textes**.
- Audit : **483 fichiers C++, 29 langues**, réussi.

Le banc build/verification-heathkit et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test ajouté au workflow GitHub. Le catalogue complet et les politiques de reconstruction restent à porter.
