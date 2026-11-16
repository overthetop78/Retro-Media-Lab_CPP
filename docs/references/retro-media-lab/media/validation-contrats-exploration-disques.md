# Contrats publics d'exploration des disques

Référence C# en lecture seule : `Contracts/Explorer/DiskImageContractData.cs`, `ExploredDiskImage.cs`, `Images/Formats/Floppy/Scp/ScpTrackContractMapper.cs`, interfaces `IDiagnostic`, `IEntree`, `IFormatDetecte`, `IImageDisquette`, `IPiste` et composition `DiskImageExplorerFactory`.

L'API `exploreDiskAsync` expose un résultat opaque `ExploredDisk`. Les instantanés publics décrivent le conteneur, les pistes, les révolutions, les secteurs, les formats, les volumes, les entrées et les diagnostics. Aucun contrat public ajouté ne dépend de MediaFileSystems. Le résultat réutilise le mapper existant pour les volumes/entrées ; l'inférence de validité propre à l'exploration disque est explicite.

Les blocs de chaque image sont matérialisés une fois et possédés par un cache partagé immutable. Les octets, tags et intervalles SCP sont conservés par propriété partagée. `selectFormat` conserve ce cache : aucune nouvelle lecture des secteurs ni reconnaissance du système de fichiers. Le volume physique de repli reste sans catalogue, comme en C#.

Fidélité : secteur absent avec données vides et numéro calculé à partir de zéro ; intégrité inconnue comptée disponible ; validité d'en-tête non renseignée dans ce contrat ; révolutions de flux en ticks avec résolution séparée et départ cumulatif en nanosecondes ; taille SCP déclarée conservée même nulle ; checksum déclaré en huit chiffres hexadécimaux. Les informations de volume sont accessibles par `DiskFormatInfo.volume`, sans dupliquer ces champs dans chaque format.

Validation : compilation Debug du projet, audit des littéraux (1 535 sources C++, 56 clés d'erreur, 29 langues), banc ciblé de projection sectorielle et SCP, compteurs de secteurs, validité des entrées, informations de volume, diagnostics avec domaine de traduction/causes et déduplication, durée de vie des octets, sélection sans nouvelle lecture, annulation et façade publique sur une petite image FAT12 temporaire. Tous ces contrôles passent. Le banc et ses médias temporaires ont été supprimés. Les codecs antérieurs n'ont pas été retestés.
