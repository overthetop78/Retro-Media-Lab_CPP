# CP/M —15novembre2026

Le banc compile25 sources C# originales, dont les 12 classes CP/M génériques/Amstrad et les contrats réels des entrées/volumes. Seule l’interface IFileSystemReader est limitée à ses quatre membres sectoriels pour isoler l’adaptateur document/volume. Les recettes de test ne construisent que les octets d’entrées, géométrie, blocs et données ; elles ne réécrivent aucun algorithme de lecture C#.

Les 25 empreintes source correspondent à l’inventaire courant. SHA-256 du manifeste ordonné chemin/empreinte : `c03e4aaef842cd526733c87661cb92a6d84671cdb060c7211b6c166b9d99888e`.

76 premiers scénarios passent sans différence contre les DLL Debug de production : huit dispositions génériques, secteurs acceptés et taille acceptée par Read seule, identifiants et casse, score minimal, minuscules/attributs, labels/mots de passe/utilisateurs, extents et allocations étroites/larges, doublons, moitié/allocation majoritairement hors limites ou manquante, secteurs absents/courts/trop longs, compte d’enregistrements nul/maximal, répertoire partiellement absent, déplacement Epson et égalité des scores, CPC système/données/recherche/fenêtre incohérente, PCW normal/vide/large/invalide et bornes de chaque champ de spécification.

Un scénario ciblé supplémentaire vérifie le regroupement de deux extents Amstrad MIXED/mixed en ordre d’extent inversé : la graphie initiale et le contenu combiné doivent être conservés. Ce scénario passe seul sans relancer les 76 premiers. L’union couvre77 cas distincts. Les comparaisons vérifient ensembles CatalogFormatIds exacts, champs de spécification, reconnaissance, nom/identifiant/capacité/espace libre du volume, toutes les entrées (nom/type/taille/commentaire/attributs/référence/validité et SHA-256 des octets) et le nombre/ordre des catégories d’avertissements.

Cinq scénarios uniques d’annulation déjà demandée couvrent reconnaissance/lecture générique, reconnaissance/lecture Amstrad et sondage CPC. Aucun défaut de lecture C++ n’a été trouvé dans les chemins exercés ; la comparaison n’établit pas l’absence de tout défaut dans le C#. Les constantes publiques sont ensuite déplacées vers CpmLayoutConstants.h sans changement de valeur, leur compilation est vérifiée sans avertissement dans le build groupé RT11 (exit0 communiqué par le parent), sans relancer les 77 contenus.

Les 29catalogues CP/M sont relus conjointement :14 premières langues par le parent,14dernières et contrôle en-US par cet agent. Les paramètres et invariant CP/M sont conservés. Aucun média réel n’est intégré, aucun test n’est placé dans le workflow GitHub et aucun bloc antérieur n’est relancé.

L’inventaire a été vérifié et intégré par le parent ; après son accord, le banc temporaire et ses artefacts sont supprimés. Le registre global et ses adaptateurs restent hors de cette validation sectorielle.
