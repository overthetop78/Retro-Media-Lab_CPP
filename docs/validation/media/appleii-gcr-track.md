# Validation du codec de piste Apple II — 15 novembre 2026

Le banc compile 36 fichiers C# originaux sans modification. Leurs SHA-256 ont été comparés à l’inventaire, dont encodeur, décodeur, format et primitives Apple II, modèles de secteurs/structures, descriptions, base d’encodage, reconstruction NRZI et génération des révolutions.

Les 1 276 cas comparent requêtes de 13/16 secteurs, pistes partielles à format explicite, nombres ambigus ou contradictoires, volumes/cylindres/faces/numéros limites, tailles erronées, durées nulles/extrêmes, métadonnées ignorées, motifs de charge utile et ordre des erreurs. Le banc compare nombre et SHA-256 des cellules binaires, index et intervalles uint32 little-endian.

Les résultats décodés sont comparés en totalité : identifiant/nom, confiance, cellule, octets, ordre/type/position/longueur/description des structures, coordonnées et données des secteurs, intégrité, tags, format, CRC enregistrés, états séparés et marque supprimée. Les cas couvrent coupures aux frontières, décalages, préfixes, mutations de bits, remplacement de prologues/champs, pistes mélangeant 5-and-3 et 6-and-2, rotation circulaire et recherche de données aux limites exactes de la fenêtre. Des cas passent aussi par génération de révolution puis décodage NRZI.

La source produit 1 151 résultats et 125 refus : 115 ArgumentException et 10 ArgumentOutOfRangeException. Les résultats contiennent au total 5 046 adresses, 4 995 structures de données et 5 046 secteurs (4 567 valides, 256 invalides, 223 indéterminés). Aucun écart ni normalisation n’est nécessaire. Le comparateur garde les catégories et paramètres des exceptions source.

Trois contrôles supplémentaires vérifient l’annulation initiale des points d’entrée ; un contrôle intègre l’encodeur et le décodeur au registre explicite, avec contenu et intégrité du secteur. Les 232 textes chargés depuis les DLL des 29 langues correspondent aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **1 512 contrôles, 0 différence, 1 276 cas, 232 textes**.
- Audit : **435 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `7dededff78f412f414da31094bd4bd4cdd9e166c6ee56097c4ecc77026c7d9c7`.

Le banc build/verification-appleii-track et ses journaux de référence/comparaison sont supprimés après validation ; aucun média utilisateur ni test ajouté au workflow GitHub. Le résultat ne valide pas les politiques de reconnaissance/reconstruction Apple ni une acquisition matérielle.
