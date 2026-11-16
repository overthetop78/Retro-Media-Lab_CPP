# Conteneur VDI

Sources C# en lecture seule : `Images/Formats/HardDisk/Vdi/VdiFormat.cs`, `VdiReader.cs`, `VdiWriter.cs`, constantes `HardDiskFormatConstants.cs`, `HardDiskImageFormatIds.cs`, `MediaImageWriterIds.cs` et enum `HardDiskImageVariant.cs`. Les empreintes des trois sources VDI correspondent à l'inventaire.

Le port utilise les sources, plages et représentations de blocs déjà présents. L'en-tête est séparé de la reconstruction des plages et du writer. VDI 1.1 autonome fixe/dynamique conserve les limites du C# : secteurs logiques de 512 octets, refus des parents différentiels sans résolveur, géométrie positive et dimensions contrôlées. Les UUID sont convertis explicitement entre la disposition Guid .NET sur disque et la notation canonique, sans dépendre de l'ordre natif de l'hôte.

Les entrées de table distinguent blocs stockés, non alloués et explicitement nuls. Les blocs stockés doivent être uniques, inférieurs au nombre alloué et contenus dans le fichier ; les octets supplémentaires par bloc sont pris en compte. Le dernier bloc logique peut être plus court. La géométrie CHS n'est conservée que lorsqu'elle ne dépasse pas la capacité logique. Les données restent accessibles à la demande depuis la source possédée par les plages.

Le writer choisit la variante explicite Fixed/Dynamic d'un document VDI ; sans choix valide, la présence de plages nulles/non allouées sélectionne Dynamic. Le parsing accepte aussi les valeurs numériques de l'enum C# et leurs espaces périphériques. L'écriture utilise des blocs de 1 Mio, omet les blocs entièrement nuls en dynamique et complète le dernier bloc physique avec des zéros. Les UUID d'image et de modification sont nouvellement générés comme en C#.

La sortie atomique commune dispose maintenant d'un producteur positionnable. Elle prépare un QSaveFile, écrit une seule fois les données, revient compléter l'en-tête et la table, puis publie seulement après réussite et contrôle d'annulation. Les producteurs séquentiels existants passent par le même mécanisme. Aucun chargement intégral d'une image disque n'est ajouté au writer.

Les erreurs de lecture VDI et KryoFlux sont ajoutées au classement des rejets du registre existant. Cette correction permet la poursuite de la reconnaissance ; l'annulation et les erreurs d'E/S continuent de se propager. L'intégration de tous les formats à la composition globale et à App reste à terminer.
