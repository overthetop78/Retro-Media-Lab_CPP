# Apple Lisa Office — 15 novembre 2026

Le banc compile25 fichiers C# originaux, dont les 12 classes Lisa et leurs vrais contrats de volumes/entrées et helpers de noms/en-têtes. Seule IFileSystemReader est limitée à ses quatre membres sectoriels pour isoler l’adaptateur document/volume. Aucun algorithme C# de lecture n’est réécrit. Les images synthétiques ne fournissent que données/tags et géométrie.

Les 25 empreintes correspondent à l’inventaire courant. SHA-256 du manifeste ordonné chemin/empreinte : `c314087be6164d51264ca3b1bbc6cca9fe62d5097226e617188dd6576e47761d`.

72 scénarios comparatifs passent sans différence : versions table/hash/B-tree et inconnues, noms de versions publics, tous les octets Latin1, en-têtes courts et longueurs limites, noms MDDF vides/blancs/Latin1/nuls/trop longs, MDDF absents ou concurrents, formats exacts/majuscules/incorrects, tags absents/tronqués/longs, masque de page, catalogues absents ou dupliqués, entrées invalides/réservées, contenus vides/de tailles différentes, lacunes et doublons, première page non nulle, ordre d’énumération inversé, compte de pages libres et grandes références logiques représentables en C#.

Les tests de valeurs blanches incluent NEL et espace insécable Latin1, ainsi qu’un nom mixte avec tabulation qui doit rester une donnée. Les comparaisons portent sur reconnaissance, volumes/capacité/espace libre, toutes les entrées (nom/type/taille/commentaire/attributs/référence/validité et SHA-256 complet du contenu), catalogues et nombre/ordre des catégories d’avertissements. Trois annulations déjà demandées vérifient canRead, read et readCatalog. Aucun défaut de code Lisa n’a été trouvé dans les scénarios exercés ; ce résultat ne prouve pas que le C# est exempt de tout défaut.

Les 29catalogues Argos ont été effectivement relus ; les 28 langues non anglaises sont corrigées, en-US identique à Constants, paramètres et invariants Lisa/MDDF/0x/${0} préservés. La production compile sans avertissement dans le build groupé parent. Le banc compile et s’exécute contre les DLL Debug stables. Aucun bloc antérieur n’est relancé, aucun test n’est ajouté au workflow GitHub et aucun média réel n’est embarqué.

Le registre global et ses adaptateurs restent hors de cette validation sectorielle. L’inventaire a été vérifié et intégré par le parent ; le banc temporaire et ses artefacts sont supprimés après son accord.
