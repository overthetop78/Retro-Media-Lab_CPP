# Validation TYCOM FM — 15 novembre 2026

Le banc compile 36 fichiers C# originaux inchangés, dont les SHA-256 correspondent à l’inventaire. Empreinte du manifeste : `86d02ad6e3590449f02f66ae5434cb936d3fb1c99429a50bf48572ff69fb0223`.

Les **1 456 cas** comprennent 325 encodages, 256 CRC, 221 flux construits, 47 lectures d’en-tête, 23 lectures de données, 480 recherches bornées, 99 lectures FM et cinq suites de cellules. Les pistes couvrent les 256 valeurs d’octet et les coordonnées limites, les états normal/supprimé, les quatre marques, les gaps 1 663/1 664/1 665, les en-têtes intermédiaires, CRC vrais/faux, données absentes ou tronquées, préfixes et cellules altérées.

La référence retourne 368 secteurs : 330 à intégrité vraie, 21 fausse, dix-sept indéterminée. Les 764 structures comprennent 400 en-têtes et 364 données. Le rejet des en-têtes au CRC incorrect est vérifié, ainsi que la distinction entre données tronquées appariées et marques non classifiées. Les helpers contrôlent la concordance de la marque attendue et de l’octet lu.

Toutes les cellules, l’index, les intervalles uint32 little-endian, la durée de cellule, la confiance, les octets, secteurs et métadonnées, structures/types/positions/longueurs/descriptions et ordre sont comparés. Les seize refus comprennent deux cylindres, deux faces, une piste vide, trois adresses TYCOM, cinq tailles, deux durées nulles et un compte FM négatif. Ce dernier remplace l’exception d’allocation C# par le diagnostic commun BitReadCount.

Six contrôles couvrent l’annulation de la lecture FM, des deux lecteurs de champs, de la recherche, du décodeur et de l’encodeur. Quatre contrôles vérifient les positions int64 extrêmes, un le compte FM int64 maximal et un les lectures vides indépendantes de leur départ. Le registre automatique explicite restitue cylindre 255, face logique zéro, secteur 255, charge utile et intégrité, avec marque supprimée. Les 261 textes des DLL des 29 langues correspondent aux catalogues relus.

- Debug et Release : **1 730 contrôles, 0 différence, 1 456 cas, 261 textes** par configuration.
- Compilation complète/déploiement : MSVC x64, Qt 6.12.
- Audit des littéraux : **536 fichiers C++, 29 langues**, réussi.
- Audit des frontières : **524 fichiers, aucune violation détectée**.
- Le banc build/verification-tycom et ses journaux sont supprimés après vérification. Aucun média utilisateur ou test dans le workflow GitHub.

Le portage global reste en cours.
