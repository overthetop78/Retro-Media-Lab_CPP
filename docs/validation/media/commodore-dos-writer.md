# Validation du writer Commodore DOS — 15 novembre 2026

19 sources C# originales sont compilées sans modifier le dépôt source. Leurs empreintes correspondent à l’inventaire. Les contrats de plans sectoriels sont des adaptateurs de données ; les algorithmes de politique, géométrie, noms, allocation et construction sont originaux.

942 cas de référence : les 256 octets de type avec trois longueurs REL chacun, un parcours des 65 536 unités UTF-16 pour les noms, noms vides/blancs/limites/caractères particuliers, trois formats standard et leurs variantes de casse, erreurs de format et de validation. Les écritures couvrent fichiers vides, frontières de 254 octets, changements de secteur latéral REL à 120 pointeurs, limite classique de 720 pointeurs et dépassement, répertoires à plusieurs secteurs, doublons, métadonnées natives, disque plein et fichiers jusqu’à 800 000 octets.

Les sorties valides comparent format, géométrie, capacité, nombre de blocs, SHA-256 des octets de tous les secteurs et SHA-256 de toutes leurs adresses ordonnées. Les erreurs comparent les catégories de refus ; les nouveaux diagnostics localisés ne sont pas forcés à reproduire les anciennes phrases anglaises d’exception.

106 plans valides sont ensuite relus par le lecteur C++ : nombre d’entrées, BAM connu, absence d’avertissements et, pour les plans à un fichier, contenu et validité des métadonnées. Un contrôle supplémentaire vérifie l’annulation avant construction. Les six clés des 29 langues sont chargées depuis les DLL et comparées aux catalogues relus, soit 174 textes.

- Compilation complète/déploiement Debug et Release : MSVC x64, Qt 6.12.
- Dans chaque configuration : **1 476 contrôles, aucune différence**, 473 cas acceptés, 469 refusés, 106 allers-retours lecteur.
- Audit des littéraux : **351 fichiers C++, 29 langues**, réussi.
- SHA-256 du manifeste C# : `56407f994ebc1cb10b4bda2b6f177e454fe656a1427c3b22c72b866dd9ebb2b7`.

Le banc temporaire `build/verification-commodore-dos-writer/` et ses journaux de comparaison sont supprimés après validation. Aucun média utilisateur ou test matériel, aucun ajout au workflow GitHub. Aucun contrôle matériel ni validation globale des trois bibliothèques n’est déduit de ces résultats.
