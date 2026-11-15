# Validation des géométries Apple — 15 novembre 2026

Le banc temporaire compile dix fichiers C# originaux : six classes de géométrie/catalogue, leurs exceptions, SectorAddress et les deux catalogues de constantes géométriques Engine/MFS. Un espace de noms vide satisfait un using Functions inutilisé ; aucun algorithme de référence n’est réécrit. Les dix empreintes correspondent à l’inventaire ; empreinte du manifeste : `48b99fdd51f70f13b780789f373092dc21a6452d878546d71d04810ab97f4879`.

Dans chaque configuration Debug/Release : **4 276 opérations comparées, 13 groupes de résultats, 0 différence**.

Couverture : tous les blocs Macintosh simple et double face, tous les blocs Lisa, tous les cylindres et changements de zones, limites négatives et supérieures, nombres de faces invalides, priorité de validation des faces avant le bloc, valeurs extrêmes signées, géométries retournées par ForHeads, capacités reconnues et leurs voisines, noms et ordre de toutes les dispositions/interprétations, permutations DOS/ProDOS et dimensions Apple II/MFM/taguées. Les erreurs sont comparées par catégorie ; les messages anglais C# ne servent pas de traduction. Le banc C++ exige les codes de diagnostic correspondants.

Les compilations complètes et déploiements Debug/Release réussissent. Aucun avertissement ou erreur du compilateur C++ dans les journaux. Les audits passent : **573 fichiers C++ pour les littéraux et catalogues**, **561 fichiers pour les frontières**, **0 violation** ; git diff --check réussi. Les 29 catalogues ont été relus et corrigés, notamment les termes cylindre, tête de lecture et bloc logique, ainsi que les invariants Apple/Macintosh.

Le banc et ses journaux sont supprimés après validation, sans ajout aux workflows. Cette validation porte sur les géométries et le catalogue ; elle ne remplace pas les comparaisons des lecteurs/writers Apple qui restent à porter.
