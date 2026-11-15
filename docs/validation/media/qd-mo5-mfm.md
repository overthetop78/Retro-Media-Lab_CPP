# Validation QD MO5 MFM — 15 novembre 2026

Le banc compile 34 fichiers C# originaux inchangés, dont les SHA-256 correspondent à l’inventaire. Empreinte du manifeste : `2b62e263a301689fcbb86276221f5328dfd37c49fd2769cbdfdf71cff8d728fa`.

Les **1 064 cas** comprennent 265 checksums, 325 pistes encodées, 174 flux construits, trente lectures d’en-tête, 25 lectures de données, 240 recherches bornées et cinq suites de cellules. Les 256 préfixes sont testés avec les 256 valeurs d’octet uniformes ; le préfixe FB est conservé dans la comparaison. Sont également couverts les secteurs 0–65 535, les attributs sectoriels et leur défaut, les octets réservés non nuls, plusieurs secteurs, durées, métadonnées ignorées, checksum incorrect, en-têtes intermédiaires, données absentes/tronquées et cellules altérées.

Les gaps 831/832/833 vérifient la limite incluse de recherche après l’en-tête. Les helpers couvrent les départs négatifs, le recouvrement du préambule, les lectures partielles et les distances nulles/négatives. Les 473 secteurs retournés par la référence comprennent 331 intégrités vraies, 94 fausses et 48 indéterminées. Les 1 568 structures comprennent 478 en-têtes et 1 090 données.

Toutes les cellules, l’index, les intervalles uint32 little-endian, la durée de cellule, la confiance, les octets, les secteurs et leurs métadonnées, les structures/types/positions/longueurs/descriptions et leur ordre sont comparés. Les dix-sept refus sont comparés par diagnostic : deux cylindres, deux faces, une piste vide, cinq plages QD MO5, cinq tailles et deux durées nulles.

Six contrôles vérifient l’annulation du checksum, des deux lectures de champ, de la recherche, de l’encodage et du décodage. Quatre contrôles supplémentaires couvrent les bornes int64 extrêmes des recherches et lectures sans accès hors limites. Le registre automatique explicite restitue le secteur 65 535 à C0/H0, sa charge utile et son intégrité. Les 203 textes des DLL des 29 langues correspondent aux catalogues relus. Une erreur d’indice dans le script ponctuel d’harmonisation des traductions a été corrigée avant leur validation finale.

- Debug et Release : **1 278 contrôles, 0 différence, 1 064 cas, 203 textes** par configuration.
- Compilation complète/déploiement : MSVC x64, Qt 6.12.
- Audit des littéraux : **531 fichiers C++, 29 langues**, réussi.
- Audit des frontières : **519 fichiers, aucune violation détectée**.
- Le banc build/verification-qd-mo5 et ses journaux sont supprimés après vérification. Aucun média utilisateur ou test dans le workflow GitHub.

Le portage global reste en cours.
