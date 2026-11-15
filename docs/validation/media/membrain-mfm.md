# Validation Membrain MFM — 15 novembre 2026

Le banc compile 35 fichiers C# originaux inchangés, dont les SHA-256 correspondent à l’inventaire. Empreinte du manifeste : `bc1a3611ad32f7c3cc92aba1d20e1768b38a31aa88312722c21d222eb7125436`.

Les **934 cas** comprennent deux comparaisons exhaustives compactées : les 8 192 adresses valides à empaqueter et les 65 536 paires d’octets à dépaqueter. S’ajoutent 112 empaquetages d’entiers signés, 322 pistes encodées, 299 flux construits, 67 lectures directes d’en-tête, 37 lectures directes de données, 90 recherches bornées et cinq suites de cellules. Les pistes couvrent les 256 valeurs d’octet et les coordonnées limites, plusieurs secteurs, les métadonnées ignorées et les erreurs de taille/adresse/durée.

Les flux couvrent F8 à FB ainsi que F7/FC rejetés, CRC vrais/faux, données absentes ou tronquées, en-têtes intermédiaires, bits hauts de cylindre inutilisés, préfixes et cellules altérées. Les gaps 703/704/705 contrôlent la limite de recherche. Les helpers vérifient aussi les octets de synchronisation différents, les marques invalides, les départs négatifs et les blocs partiels.

La référence retourne 558 secteurs : 338 à intégrité vraie, 119 fausse, 101 indéterminée. Les 1 103 structures comprennent 582 en-têtes et 521 données. Tous les bits, index, intervalles uint32 little-endian, durées de cellule, confiance, octets, secteurs et métadonnées, structures/types/positions/longueurs/descriptions et ordre sont comparés. Les seize refus sont comparés par diagnostic : deux cylindres, deux faces, une piste vide, trois adresses Membrain, six tailles de secteur et deux durées nulles.

Cinq contrôles vérifient l’annulation des lecteurs d’en-tête/données, de la recherche, du décodeur et de l’encodeur. Deux gardes C++ supplémentaires vérifient les bornes int64 extrêmes de recherche sans débordement. Le registre automatique explicite restitue cylindre 255, face un, secteur quinze, charge utile et intégrité. Les 203 textes des DLL des 29 langues correspondent aux catalogues relus.

- Debug et Release : **1 145 contrôles, 0 différence, 934 cas, 203 textes** par configuration.
- Compilation complète et déploiement : MSVC x64, Qt 6.12.
- Audit des littéraux : **526 fichiers C++, 29 langues**, réussi.
- Le banc build/verification-membrain et ses journaux sont supprimés après vérification ; aucun média utilisateur ou test dans le workflow GitHub.

Cette validation porte sur le codec. Le portage global reste en cours.
