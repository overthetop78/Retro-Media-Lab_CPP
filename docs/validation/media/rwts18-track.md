# Validation des pistes RWTS18 — 15 novembre 2026

Le banc compile 37 fichiers C# originaux inchangés : encodeur/décodeur/format/primitives RWTS18, table et lecture Apple, modèles, descriptions, base d’encodage, reconstruction et génération des révolutions. Leurs SHA-256 correspondent à l’inventaire.

Les 1 206 cas incluent les 720 permutations des six secteurs, des motifs déterministes, faces/identifiants et coordonnées limites, tailles et dispositions erronées, durées nulles/extrêmes et métadonnées ignorées. Des erreurs simultanées vérifient la priorité des validations. Les encodages comparent nombre et SHA-256 des cellules, index et intervalles uint32 little-endian.

Le décodage compare identifiant/nom, confiance, cellule, octets, ordre et description complète des structures, coordonnées/tailles/positions/contenus/intégrité et toutes les métadonnées absentes des secteurs. Les mutations touchent marque, adresse, XOR, fin, identifiant, symboles et checksum. Des coupures aux frontières, rotations de piste, préfixes, suppressions et insertions de bits testent la fenêtre complète et les positions déclarées après lecture synchronisée. Des cas passent par révolution puis décodage NRZI.

La référence produit 1 175 résultats et 31 refus (16 ArgumentException et 15 ArgumentOutOfRangeException). Les résultats comprennent 6 619 structures d’adresse et 6 540 structures de données, avec 6 619 secteurs : 6 514 valides, 26 invalides et 79 indéterminés. Aucune normalisation n’est appliquée.

Trois contrôles supplémentaires couvrent l’annulation à l’entrée ; un contrôle passe une piste complète par le registre automatique explicite, avec vérification du contenu et de l’intégrité des six secteurs. Les 116 textes chargés depuis les DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Chaque configuration : **1 326 contrôles, 0 différence, 1 206 cas, 116 textes**.
- Audit : **441 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `e2f1b1796c42d11c0f1c5be62e61a95d21e679e9201e15e770a1c70cd8036bd5`.

Une première exécution du comparateur ajoutait une clé JSON lors de sa lecture, supprimant les secteurs par défaut de 440 requêtes. Cette erreur du banc a été corrigée avant la comparaison finale ; les sources de production n’ont pas été adaptées à ce défaut.

Le banc build/verification-rwts18-track et ses journaux sont supprimés après vérification. Aucun média utilisateur ni test ajouté au workflow GitHub. Ces résultats ne valident pas les politiques de reconstruction Apple ni une acquisition matérielle.
