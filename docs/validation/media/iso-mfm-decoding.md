# Validation décodage ISO MFM — 15 novembre 2026

37 sources C# originales compilées sans modifier leur dépôt ; leurs empreintes correspondent à l’inventaire. Le banc produit 1 292 cas : 265 lectures MFM, 265 brutes, 265 FM32, 9 reconnaissances de marques, 17 motifs variables, 9 motifs seize bits, 9 motifs d’octets, 30 séquences MFM, 340 décodages de cellules, 53 décodages de révolutions, 8 lectures d’en-tête, 8 lectures de données et 14 sélections PLL contrôlées. Cinq nombres négatifs sont refusés ; les autres cas conservent leurs résultats, y compris les absences et champs incomplets.

Les 256 valeurs d’octet sont vérifiées dans les trois représentations. Les plages couvrent les offsets négatifs/int32 extrêmes, longueurs invalides et frontières. Les décodeurs couvrent toutes les valeurs de marque, les huit tailles ISO, données supprimées, CRC valides/invalides/absents, CHRN atypiques, troncatures au niveau des champs, rotations, erreurs de cellule, queue circulaire de part et d’autre de 20 000 cellules, pistes physiques/logiques, temporalités et jitter. Les essais PLL contrôlent les sept scores, l’ordre, les égalités et le retour anticipé central.

La comparaison porte sur l’identité, confiance et temporalité ; longueur/SHA-256 des octets et payloads ; totalité des secteurs, offsets, tailles, intégrités nulles/vraies/fausses et marque supprimée ; totalité des structures, leurs offsets/longueurs et leurs descriptions anglaises. Les lecteurs de champs comparent également leurs données et CRC. Les erreurs comparent leur catégorie. L’annulation initiale est contrôlée pour décodage de cellules/révolutions, lecture de séquence et sélection PLL ; une exception du rappel PLL est propagée.

Les sept textes des 29 langues sont chargés depuis les DLL déployées et comparés aux catalogues relus : 203 contrôles. Le banc Ninja reprend le préfixe d’inclusion MSVC français du build principal.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **1 497 contrôles, aucune différence**, 1 292 cas C#.
- Audit : **375 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `d79a6c8742551a0a7badfa4d145c101f100ae9a0777e3c72be7d4b8f4af14027`.

Le banc `build/verification-iso-mfm-decoding/`, ses exécutables, références et journaux de comparaison sont supprimés après validation. Aucun média utilisateur, test matériel ou ajout au workflow GitHub. La reconstruction SCP 1581 n’est pas couverte par ce bloc.
