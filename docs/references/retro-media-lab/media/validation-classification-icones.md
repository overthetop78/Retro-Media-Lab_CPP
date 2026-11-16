# Classification et icônes

Sources C# en lecture seule : `DiskClassificationCatalog`, `DiskVisualizationClassificationFunctions`, `DiskVisualizationDefaults`, `DiskVisualizationClassification`, `DiskMediaCategory`, `MediaIconSelector`, `MediaIconIds`, `MediaIconExtensions` et `GwVisualizationPolicy`.

Le catalogue possède ses formats, exclut exactement l'identifiant `raw.scp`, conserve les familles distinctes sans tenir compte de la casse et les trie selon la locale courante avec le collateur Qt. Les recherches exactes précèdent les six alias Apple/Mac. Les protections sont filtrées sur les identifiants canoniques ; le nom propre Brøderbund RWTS18 reste invariant.

La résolution de visualisation conserve la distinction entre absence de machine en mode automatique et choix manuel. Une protection fournie, même vide, remplace le décodeur par défaut. Les marqueurs de densité et ensembles de machines sont ceux du C# ; aucune machine n'est déduite d'un format seul.

Le choix d'icône privilégie le type connu et le format physique. Les replis par extension conservent la liste exacte du C#, notamment `.gdi` non retenu comme optique sans type explicite. Un helper de suffixe conserve `Path.GetExtension` pour les fichiers commençant par un point et ceux terminant par un point ; les deux cas ont aussi été vérifiés avec `[System.IO.Path]` sur le poste.

La décision de conversion SCP utilise le format détecté et les capacités logicielles GW existantes : cas DSK/EDSK, cas ATR 90/130 avec sortie IMG, argument GW et définitions intégrées. Elle ne prétend pas mesurer les capacités matérielles d'un lecteur. La classe de résultat de détection C# n'est pas nécessaire à cette décision : seul son format est utilisé, reçu directement par l'API C++.

Validation : compilation Debug, audit des littéraux, contrôle ciblé de catalogue possédé, priorité exacte/alias, familles/protections, choix automatique/manuels, densités, toutes les branches d'icônes et règles SCP. Tous ces contrôles passent, sans lecture ni écriture de média. Le banc temporaire a été supprimé. Aucun changement du dépôt C# ni de l'affichage App.
