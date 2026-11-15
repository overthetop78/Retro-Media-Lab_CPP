# Sources et partitions — référence du portage

Relevé du 15 novembre 2026, C# consulté en lecture seule.

## Sources C# utilisées

- `RetroMediaLab.MediaEngine/Images/Reading/Sources/FileRandomAccessData.cs` et `MemoryRandomAccessData.cs` : longueur connue, copie de mémoire, lectures exactes bornées, annulation, fermeture du flux fichier à chaque opération.
- `RetroMediaLab.MediaFileSystems/Interfaces/IMediaRandomAccessData.cs` et `IMediaBlockRepresentation.cs` : accès en octets, capacité, taille et nombre de blocs.
- `RetroMediaLab.MediaFileSystems/Contracts/MediaVolumeDescriptor.cs`, `MediaVolumeDetectionResult.cs`, `Constants/MediaVolumeOrigins.cs` et `PartitionTableIds.cs` : résultats, origines et identifiants.
- `RetroMediaLab.MediaFileSystems/Exploration/Partitioning/MbrVolumeDetector.cs` : quatre entrées primaires, partitions étendues 05/0F/85, liens EBR relatifs à leur base, maximum 4 096 enregistrements, boucles, bornes et chevauchements.
- `GptVolumeDetector.cs` dans le même dossier : entêtes primaire/secondaire, CRC32, tableaux limités à 128 MiB, GUID, noms UTF-16LE et bornes des partitions.
- `WholeMediaVolumeDetector.cs` : dernier recours sur la longueur adressable.
- `RetroMediaLab.MediaFileSystems/Functions/Crc32Functions.cs` : CRC32 réfléchi, polynôme EDB88320.

Le banc C# compile les fichiers originaux par liens ; aucun fichier du dépôt C# n’est écrit. Les empreintes restent dans l’inventaire.

## Découpage C++ actuel

Les interfaces de source et de blocs appartiennent à MediaFileSystems. MediaEngine implémente les sources fichier/mémoire dans ses fichiers privés `src/Sources/`, sans les exposer à App par ses en-têtes publics. La dépendance CMake MediaEngine → MediaFileSystems est privée. Les détecteurs et contrats de volumes appartiennent à MediaFileSystems. Aucun algorithme de partitions dans App ou Infrastructure.

Les lectures C++ de ce bloc sont synchrones et annulables via stop_token ; elles ne créent pas de thread. Leur appel depuis une opération d’interface devra être pris en charge par le service d’opérations de MediaEngine, pas exécuté par App sur le thread graphique. Ce service reste à porter.

Le média par blocs a une longueur connue et stable pendant sa durée de vie. Il ne remplace pas encore les documents complets, représentations sectorielles avec secteurs absents/dégradés, flux, optique ou séquentiel du C#. La découverte de volumes pour ces représentations reste à porter.

## Contrôles supplémentaires

- Les plages sont validées par soustraction avant tout accès ou multiplication.
- Une taille de bloc nulle ou supérieure à 1 MiB est rejetée avant allocation. Les tailles habituelles 512/4096 sont testées.
- Le contrôle GPT commence par `tailleTableau > capacité` avant la soustraction. Dans le C#, une division entière signée d’un résultat légèrement négatif pouvait tronquer vers zéro, autoriser le tableau à LBA 0, puis provoquer une exception de lecture. Le C++ renvoie le diagnostic GPT correspondant avant de lire.
- Les erreurs d’accès des sources et l’annulation ne sont pas converties en simples diagnostics de structure GPT/EBR. Une erreur système de lecture est distincte d’une fin de fichier.
- Le flux fichier est local à chaque lecture ; RAII le ferme sur succès, erreur et annulation. La copie mémoire est privée et immuable.

## Traductions

Domaine `MediaFileSystems`, 36 diagnostics sous `diagnostics.json`, Constants et 29 langues. Les valeurs techniques MBR, EBR, GPT, CRC et LBA sont partagées dans le fichier d’invariants commun.

Le script Argos essaie la phrase complète avant les protections de secours si les invariants restent identiques. Cela évite de découper inutilement les phrases techniques en fragments. « Storage medium » précise le contexte de support de stockage dans la base anglaise. Le libellé d’annulation et le nouveau diagnostic de lecture ont été régénérés seuls dans des catalogues temporaires puis fusionnés, afin de conserver les autres traductions déjà revues. Des contresens en français et des sorties restées en anglais en arabe/espagnol ont été corrigés à la relecture. Paramètres et invariants sont contrôlés dans chaque langue.
