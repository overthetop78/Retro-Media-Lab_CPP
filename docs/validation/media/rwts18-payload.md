# Validation de la charge utile RWTS18 — 15 novembre 2026

Le banc compile neuf fichiers C# originaux non modifiés : AppleRwts18Codec, AppleRwts18Format, AppleIIGcrFormat, identifiants/noms des codecs, TrackSector, BitPrimitives, SectorSizeCode et BitConstants. Leurs SHA-256 correspondent à l’inventaire.

Les 5 492 cas comprennent 1 248 encodages valides, neuf tailles d’encodage refusées, 4 227 décodages valides en longueur et huit longueurs de décodage insuffisantes. Les données comprennent tous les remplissages constants d’un octet et de chacune des trois pages, des bits isolés aux frontières, 128 charges pseudoaléatoires déterministes, les 256 valeurs possibles à quinze positions de symboles et de checksum, les valeurs brutes jusqu’à FF, et 128 jeux numériques déterministes. Les longueurs supplémentaires vérifient qu’elles sont ignorées.

Les encodages comparent tous les 1 025 symboles, les trois pages décodées et le checksum ; les décodages comparent les 768 octets complets et l’intégrité. Les 1 248 encodages retournent une intégrité vraie après conversion inverse de la table. Les décodages directs retournent 71 intégrités vraies et 4 156 fausses, avec données conservées. Les erreurs sont comparées par catégorie/paramètre ; le diagnostic C++ de longueur insuffisante est associé explicitement à ArgumentOutOfRangeException(index), avec vérification préalable de la taille décrite dans la référence.

Deux contrôles vérifient l’annulation des points d’entrée, y compris avant les validations de taille. Les 58 textes chargés depuis les DLL des 29 langues sont comparés aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **5 552 contrôles, 0 différence fonctionnelle, 5 492 cas, 58 textes**.
- Audit : **438 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste : `fbd795cd2eb3d0c5a1f013f27f3dde4964f995199b8bdd3bff6eda4c25827f55`.

Le banc build/verification-rwts18-payload et ses journaux de référence/comparaison sont supprimés après validation. Aucun média utilisateur ni test ajouté au workflow GitHub. Cette validation concerne les transformations de charge utile ; elle ne valide pas encore les codecs de piste RWTS18.
