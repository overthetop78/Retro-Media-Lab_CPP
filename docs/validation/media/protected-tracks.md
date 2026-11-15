# Validation des pistes protégées — 15 novembre 2026

Le banc compile trente sources C# originales sans modification ni remplacement algorithmique. Leurs empreintes SHA-256 correspondent à l’inventaire.

Les 1 101 cas comprennent 442 constructions de piste, 30 images, 518 adaptations SCP, 75 adaptations de pistes encodées et 36 séries de fenêtres (18 SCP et 18 protégées). Ils comparent toutes les coordonnées, bits, timings, structures/descriptions, caractéristiques/descriptions, intervalles, index, résolutions, protection et ordre des pistes/fenêtres. 661 cas retournent un résultat et 440 sont refusés ; catégories/paramètres d’exception et message de doublon sont comparés.

La matrice couvre bits absents/vides, révolutions absentes, plages des trois types de métadonnées, durées non finies, coordonnées/résolutions int32 extrêmes, les 256 flags et les 256 résolutions SCP, permutations et doublons multiples, encodage à durée nulle et débordements des fenêtres. Les résolutions mélangées confirment la conservation des intervalles sans rééchantillonnage.

Six contrôles supplémentaires vérifient propriété après mutation des arguments/conteneurs, rejet d’un bit C++ hors 0/1, annulation initiale des quatre points d’entrée, annulation pendant les fenêtres et propagation des exceptions du callback. Les 261 textes sont chargés depuis les DLL des 29 langues et comparés aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **1 368 contrôles, aucun écart**, 1 101 cas et 261 textes.
- Audit : **402 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `2372fab560483056df55474fec631cbff9eb2d333a91614af4f07439c8a13456`.

Le banc build/verification-protected-tracks et ses journaux de comparaison sont supprimés après validation. Aucun média utilisateur, test matériel ou ajout au workflow GitHub. La collecte/cache ISO n’est pas validée par ces résultats.
