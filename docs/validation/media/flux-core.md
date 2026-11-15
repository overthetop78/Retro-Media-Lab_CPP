# Validation des primitives de flux et GCR — 15 novembre 2026

Dix sources fonctionnelles C# originales sont compilées sans modification ; leurs empreintes correspondent à l’inventaire. Le banc utilise aussi trois dépendances C# de compilation sans prétendre les porter. Les entrées sont synthétiques, temporaires et ne nécessitent aucun matériel.

## Couverture

5 734 cas de référence : 1 927 reconstructions adaptatives, 975 PLL, 1 024 octets GCR bruts, 840 encodages/écritures GCR, 272 chemins de décodage FM/MFM/NRZI, 168 créations de révolution, 128 demi-octets, 126 extensions circulaires, 102 estimations de cellule, 100 plages GCR et 72 calculs de confiance.

Les bits et intervalles sont comparés exactement, ainsi que tous les octets, checksums, cibles d’écriture et offsets finaux. Les durées/confiances finies utilisent une tolérance relative de 1e-12 ; les NaN/infinis sont comparés par état. Les cas couvrent les tableaux vides, intervalles nuls, index exclu/repli, bruit, distributions courtes/longues, arrondis au pair, bornes int32/uint32, durées non finies, limites de PLL nulles/négatives, toutes les valeurs d’octet et tous les codes GCR de dix bits, pas zéro/un/deux/trois, plages tronquées et dépassements de durée de révolution.

La première comparaison a montré 43 différences sur la conversion entière des grands intervalles. La saturation observée en .NET 10 a été reprise ; deux différences restantes concernaient la limite nulle/négative de la PLL, également alignée sur le C#. Le banc complet passe ensuite sans différence.

Les vérifications C++ supplémentaires contrôlent l’annulation avant mutation de la cible GCR, l’annulation des deux reconstructions et de la création de révolution, les bits non binaires, coordonnées négatives et calculs de plage débordants. Les huit messages des 29 langues, soit 232 textes, sont chargés depuis les DLL installées et comparés aux catalogues relus.

## Résultats

- Compilation complète et déploiement Debug et Release réussis avec MSVC x64/Qt 6.12.
- Dans chaque configuration : **11 707 contrôles, aucune différence et 232 textes traduits**.
- Audit des littéraux : 301 fichiers C++, 29 langues ; `git diff --check` passe.
- SHA-256 du manifeste des résultats C# : `138625e7cf542d4b45285b23c269c6c65eef122c9cb0da4768350ad5c0f11218`.

Le banc `build/verification-flux-core/`, ses fixtures, projets, exécutables, références et journaux temporaires sont supprimés après validation. Aucun média utilisateur, test matériel ni ajout aux workflows GitHub. Ces résultats ne valident pas encore le codec de piste Commodore 900 ni les autres codecs physiques.
