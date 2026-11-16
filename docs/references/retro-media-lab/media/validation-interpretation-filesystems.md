# Interprétation des systèmes de fichiers

Références C# en lecture seule : FileSystemAlternativePolicy.cs, FileSystemEvidence.cs et FileSystemInterpretationIdentity.cs. Les fonctions publiques natives sont regroupées dans Interpretation.h ; les séparateurs d’identité et les seuils sont partagés par le contrat et le registre existant.

La crédibilité accepte les avertissements proportionnés au catalogue ou au moins trois entrées réellement nommées et validées. Les entrées enfants sont comptées ; le parcours de comptage est itératif. Les preuves comparent successivement le nombre d’entrées, le nombre d’avertissements puis le score du décodage, sans remplacer le score fourni. L’identité brute conserve la casse et trie les noms comme le C# ; le registre applique ensuite la clé ordinale insensible à la casse pour sa déduplication, comme auparavant. Aucun second calcul d’identité n’est créé.

Validation ciblée Debug réussie : seuil de trois avertissements, bascule à quatre, noms synthétiques/invalides/blancs, enfants, priorités de comparaison, égalité stricte, identité stable malgré l’ordre d’insertion, chemins imbriqués et familles distinctes. Le catalogue courant conserve 21 lecteurs. Les sources et l’architecture de dépendances restent inchangées : MFS utilise MediaAnalysis et n’importe pas MediaEngine.

L’inventaire avait laissé des classes regroupées à l’état non porté. Quinze correspondances existantes ont été revues avant correction, en plus des trois fonctions d’interprétation désormais portées :
- FileSystemReaderCatalog.cs et SectorFileSystemRegistry.cs : Catalog.cpp, Registry.cpp et SectorRegistry.h ; ordre des 21 lecteurs, sélection par nom ou format, collecte des corruptions et déduplication des interprétations.
- FileSystemMatch.cs, FileSystemReadFailure.cs, FileSystemReadReport.cs et SectorFileSystemCandidate.cs : valeurs et collections possédées dans SectorRegistry.h.
- AtariBootDiskFileSystemReader.cs, AtariBootDiskFunctions.cs, AtariBootDiskConstants.cs et AtariBootDiskFormatCatalog.cs : BootDisk/Reader.cpp, BootDisk/Constants.h et les neuf formats partagés d’AtariMetadata. Secteurs complets, limites d’adresse et exclusion des trois catalogues contrôlées dans la source.
- AtariKFileFileSystemReader.cs, AtariKFileFunctions.cs, AtariKFileConstants.cs et AtariKFileFormatCatalog.cs : KFile/Reader.cpp et ses constantes, mêmes neuf formats, trois secteurs de démarrage, adresse et saut KBoot, longueur 24 bits et marqueur exécutable.
- AtariClkGraphicsLibraryFileSystemReader.cs : ClkGraphicsLibrary/Reader.cpp et ses constantes ; répertoire, enregistrements actifs, chaîne bornée/visitée, contenu tronqué et métadonnées conservées.

Ces correspondances existantes ont fait l’objet d’une revue statique actuelle, pas d’une nouvelle batterie de tests des lecteurs inchangés. Les empreintes des sources C# revues sont actualisées dans le CSV. FileSystemRegistryExceptions reste partiel : ses trois validations sont traduites dans Diagnostics.cpp ; UnknownReaderId n’a pas de fabrique publique correspondante. Aucun statut global de fin de MediaFileSystems n’est déclaré.

Les services génériques de migration, d’autres contrats non revus et le raccordement aux façades Engine restent dans l’objectif général. Le test temporaire a été supprimé après résultat.
