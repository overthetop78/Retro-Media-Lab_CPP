# Validation des primitives GCR Apple II — 15 novembre 2026

Le banc compile six fichiers C# originaux inchangés : AppleIIGcrCodec, AppleIIGcrFormat, AppleBitLatch, BitPrimitives, SectorSizeCode et BitConstants. Leurs SHA-256 correspondent à l’inventaire.

Les 7 574 cas comprennent une matrice des 256 encodages 4-and-4 et des 65 536 paires brutes décodées, 125 adresses limites, 3 922 lectures synchronisées et 3 526 opérations sectorielles. Les lectures croisent préfixes, positions, comptes, troncatures et curseurs partiels. Les secteurs couvrent les 256 remplissages constants, motifs déterministes, bits isolés, mutations de tous les symboles à plusieurs positions, coupes, décalages et tailles incorrectes.

La référence retourne 7 535 résultats et refuse 39 entrées. Pour les opérations sectorielles : 926 blocs valides, 473 blocs avec checksum faux et données conservées, 2 107 absences de résultat et 20 erreurs. Le comparateur vérifie octets complets, curseur, intégrité et catégorie/paramètre d’exception ; aucune normalisation de différence n’est appliquée.

Deux contrôles supplémentaires passent par une révolution réelle construite puis décodée en NRZI pour les modes 5-and-3 et 6-and-2. Un contrôle groupé vérifie l’annulation des cinq points d’entrée. Les 87 textes chargés depuis les DLL des 29 langues sont comparés aux catalogues relus.

- Compilation complète et déploiement Debug/Release : MSVC x64, Qt 6.12.
- Chaque configuration : **7 664 contrôles, 0 différence, 7 574 cas, 87 textes**.
- Sept contrôles Python distincts valident paramètres accolés/répétés, invariants imbriqués, limites de mots, protection des nombres et repli lorsque les tokens sont altérés.
- Audit : **432 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste : `af063085f23a6dd0df2b5fc8702879c541a89c03dc35e7825ec1fe4c4f0520a0`.

Le banc build/verification-appleii-primitives et ses journaux de comparaison sont supprimés après vérification. Aucun média utilisateur ni test ajouté au workflow GitHub. Ces résultats concernent les primitives ; ils ne valident pas encore le codec de piste Apple II complet.
