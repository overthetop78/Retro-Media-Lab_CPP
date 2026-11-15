# Atari DOS — 15 novembre 2026

Le banc compile 27 fichiers C# originaux : les 14 classes Atari/Dos et les contrats/définitions réels des volumes et entrées. Seule l’interface IFileSystemReader est limitée à ses quatre membres sectoriels pour isoler l’adaptateur document/volume. Les images synthétiques fournissent des blocs et des champs, sans réécriture des algorithmes de référence.

Les 27 empreintes source correspondent à l’inventaire courant. SHA-256 du manifeste ordonné chemin/empreinte : `dcbd17d36df1f4032cf939ed75bf143f3fc98ea6210233a477990abc7b8fa211`.

106 scénarios C#/C++ passent sans différence : ensembles de formats exacts et préfixe ATR, VTOC standards/étendus et leurs bornes, secteurs 128/256 et liens MyDOS complets, noms ASCII/nuls/espaces blancs, drapeaux verrouillés/supprimés/ouverts/inconnus, fichiers vides, secteurs manquants/tronqués, longueurs utiles invalides, fins prématurées/cycles, vues bornées/partagées, propriétaires incohérents, propriétaire limité à une chaîne plus courte, répertoires déplacés et sélection meilleur score/égalité, catalogue complet de 64 entrées, API sectorielle et compte libre optionnel.

Le contrôle compare les volumes (identifiant, capacité, espace libre, attributs), chaque entrée (nom/type/taille/drapeaux/référence, metadataValid, dataValid, attributs, ordre des catégories de diagnostics et SHA-256 du contenu), les résultats publics de chaîne (secteurs, propriétaires optionnels, continuation, état complet), les emplacements de répertoire, les sondages, le nombre et l’ordre des avertissements. Le banc donne volontairement integrityValid=false aux blocs synthétiques : le port doit préserver la règle source au lieu d’inventer un rejet CRC dans ce lecteur.

Cinq annulations déjà demandées passent : reconnaissance, lecture du volume, recherche de répertoire, lecture de fichier et lecture de répertoire. Les 106 comparaisons ne sont pas rejouées après succès. Aucun défaut de résultat C++ n’a été trouvé dans les scénarios exercés ; ce résultat n’affirme pas que le C# est exempt de défauts. Deux inclusions privées incorrectes sont corrigées avant le build réussi, puis tous les includes locaux sont vérifiés sur leurs chemins réels.

La production et le banc natif compilent sans avertissement dans le build Debug groupé parent. Les 29 catalogues ont été relus conjointement, dont les 14 premières langues par le parent et les 14 dernières par cet agent ; en-US correspond à Constants. Les paramètres et invariant Atari DOS sont contrôlés. Le diagnostic de vue bornée précise que le compte vient de l’entrée de répertoire ; il ne désigne pas le nombre de secteurs du catalogue. Aucun média réel n’est intégré et aucun test n’est ajouté au workflow GitHub.

Le registre global et ses adaptateurs restent hors de cette validation sectorielle. L’inventaire a été vérifié et intégré par le parent ; après son accord, le banc temporaire et ses artefacts sont supprimés.
