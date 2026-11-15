# Validation AED 6200P MFM — 15 novembre 2026

Le banc compile 36 fichiers C# originaux en lecture seule. Leurs SHA-256 correspondent à l’inventaire et restent inchangés en fin de vérification. L’empreinte du manifeste est `603d35f8229165180dfc0fb4dedfb4a21876672310cf83f0177a31356f2ba5a1`.

Les **1 204 cas** comprennent 330 constructions d’identité par appel de la méthode privée originale, 300 encodages complets, 569 flux construits avec les primitives C# originales et cinq suites de cellules. Les encodages couvrent les 256 valeurs d’octet, les marques normale/supprimée, les frontières de taille jusqu’à 65 535, le refus à 65 536, plusieurs secteurs dans l’ordre fourni, les limites d’adresse et de durée, ainsi que les métadonnées ignorées.

Les flux construits couvrent les quatre marques C0/C1/C2/C3, CRC d’en-tête et de données vrais/faux, tailles nulles et divergences de taille déclarée/charge présente, données absentes, en-têtes répétés, préfixes, cellules altérées et troncatures. Les gaps 703/704/705 cellules contrôlent la borne réelle de recherche après un en-tête de 112 cellules, dans la fenêtre de 832 cellules. Les marques non appariées et les en-têtes intermédiaires sont comparés.

La référence retourne 803 secteurs : 456 à intégrité vraie, 210 fausse et 137 indéterminée. Les 1 595 structures comprennent 839 en-têtes et 756 données. Le banc compare toutes les cellules, l’index et tous les intervalles uint32 little-endian, puis durée de cellule, confiance, octets décodés, secteurs et toutes leurs métadonnées, structures/types/positions/longueurs/descriptions et ordre de production. Aucun écart ni normalisation n’est nécessaire.

Les douze refus sont comparés par diagnostic : cinq champs AED, deux cylindres, deux faces, une piste vide, une durée de cellule nulle et une durée d’index nulle. Trois contrôles vérifient l’annulation de la construction d’identité, de l’encodage et du décodage. Deux contrôles passent par le registre explicite puis son décodage automatique, avec cylindre 255, secteur 254, face physique un/logique zéro, 257 octets et marque supprimée.

Les 174 textes chargés depuis les DLL des 29 langues correspondent aux catalogues relus. La première compilation a révélé un avertissement de conversion size_t/int32 dans une constante ; une conversion explicite de la taille fixe du motif le corrige. Les définitions de marques sont des constantes nommées compatibles avec l’audit existant.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Chaque configuration : **1 383 contrôles, 0 différence, 1 204 cas, 174 textes**.
- Audit des littéraux : **514 fichiers C++, 29 langues**, réussi.
- `git diff --check` : réussi.

Le banc `build/verification-aed6200p/` et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test dans le workflow GitHub. Cette validation concerne le codec, pas l’ensemble du portage.
