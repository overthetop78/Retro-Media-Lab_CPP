# Référence C# — secteurs et famille FAT12

Source en lecture seule : `F:/GW GUI/src/RetroMediaLab.MediaFileSystems/FileSystems/Fat12/` (27 fichiers), `Exploration/FileSystemEntryAnalyzer.cs`, `Exploration/MediaFileSystemFamilyResolver.cs`, contrats de secteurs/volumes/migration, et `RetroMediaLab.MediaEngine/Images/Models/Sectors/`.

## Répartition C++

- Primitives binaires communes : `MediaFileSystems/src/Primitives/`, partagées avec MBR/GPT. Les helpers spécifiques aux partitions restent dans Partitioning.
- BPB, dates, noms et labels FAT : `FileSystems/Fat/BootAndNames.cpp` et `Constants.h`.
- FAT12 : `FileSystems/Fat/Fat12/` ; Layout (BPB et historiques), Tables (copies, chaînes et lectures sectorielles), Directory, Reader, Writer. Les 28 associations de formats et cinq dispositions historiques restent explicites dans FormatCatalog.h.
- Contrats publics de MFS : SectorImage, FileSystemModels, Migration, Fat et Fat12. L’implémentation propriétaire/paresseuse des images est privée à MediaEngine, dans Models/Sectors ; les modèles préservent les métadonnées physiques enregistrées sans les déduire.
- Exploration : MFS résout la famille et appelle MediaAnalysis. Les identifiants retournés n’exposent pas les types MediaAnalysis. Le pont source sectorielle → FAT12 → classification reste sous MediaEngine/src/Exploration, hors de son API publique App.

Les variantes DOS Plus 800 et JFD à secteurs de 1024 octets restent distinctes des FAT à secteurs de 512 octets. Apricot conserve sa disposition historique et son descripteur dans la première FAT. Les labels de répertoire sont prioritaires, avec repli sur le BPB pour les variantes appropriées. Les dates restent indépendantes de la culture et du système hôte.

## Compatibilité et fiabilité

- Les dispositions historiques utilisent les deux copies pour calculer la racine, mais n’annoncent qu’une copie lisible, comme le constructeur C# actuel. Cette particularité reste conservée.
- Les copies FAT sont classées par plausibilité, dans l’ordre en cas d’égalité. Le C++ préfère en plus une copie dont l’intégrité enregistrée n’est pas invalide. Les données restent inspectables si toutes les copies sont dégradées, avec validité/diagnostics explicites et espace libre non garanti.
- Les secteurs absents ou de mauvaise taille gardent leur position dans la plage décodée. Ils ne sont pas supprimés par concaténation. Les métadonnées et contenus héritent de cette invalidité.
- Les chaînes cycliques/hors limites sont arrêtées. Les répertoires réutilisant un cluster déjà parcouru ne sont pas développés à nouveau. Le budget global d’extraction est borné par l’espace des clusters et un plafond de 65 535 × 512 octets ; une corruption avec liens croisés ne provoque pas des allocations répétées non bornées.
- Une entrée FAT tronquée rapporte son numéro réel, alors que le paramètre de sortie du C# pouvait ramener ce numéro à zéro.
- Le writer ne fait qu’un plan de secteurs en mémoire. Aucun matériel n’est écrit. Il conserve l’allocation consécutive, les deux copies FAT, les entrées spéciales, attributs, dates, label et octets du BPB du C#.
- Le calcul de taille FAT du writer est monotone et borné par la capacité au lieu de pouvoir osciller entre deux valeurs. Les géométries hors du domaine FAT12 sont rejetées avant allocations importantes. Les noms doublons, fichiers avec enfants, dates invalides et arbres trop profonds sont refusés.
- Les secteurs disponibles sont visités dans l’ordre d’insertion. Les images adossées aux sources lisent paresseusement et distinguent secteurs partiels/absents. L’adaptation d’observation WithDataAccess du modèle C# reste explicitement non portée dans l’inventaire ; aucun faux observer n’est ajouté.

## Traductions

23 diagnostics, base anglaise dans `Constants/MediaFileSystems/fat12.json`, puis 29 langues via `translate.py --all --files MediaFileSystems/fat12.json`. FAT12, FAT et BPB ont rejoint les invariants communs. Relecture des fichiers et corrections après Argos : phrases restées anglaises, sens inversé de l’invalidité, termes sectoriels, place des paramètres, marqueur résiduel chinois et distinction chinois simplifié/traditionnel. Le service n’est pas appelé depuis les algorithmes ; il résout les clés au niveau consommateur.

## Périmètre restant

Ce bloc ne porte ni la reconnaissance/lecture des conteneurs, ni les flux, ni les services complets de migration/conversion, ni la façade destinée à App. Les autres familles de systèmes de fichiers restent à traiter à partir de leurs sources C#.
