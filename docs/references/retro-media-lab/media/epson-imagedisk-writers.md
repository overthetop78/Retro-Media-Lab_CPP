# Writers Epson brut et ImageDisk

Références C# lues : EpsonQx10RawImageWriter.cs, EpsonQx10ConversionService.cs et les entrées EpsonQx10/ImageDisk et WriteEpsonAsync de MediaWritingComposition.cs.

Le writer brut suit le catalogue Epson variable en cylindre/face/numéro, indépendamment des indices logiques du document. Les pistes sans secteurs n'ajoutent aucun octet. Le profil source et les dimensions doivent correspondre à la cible ; doublons physiques, secteurs absents et tailles incorrectes provoquent des erreurs typées traduites. L'écriture atomique commune nettoie les sorties incomplètes.

L'adapter Epson expose les six profils natifs et IMG/IMD. IMG utilise le backend brut ; IMD restaure les métadonnées détaillées, ou construit un conteneur Epson seulement si ces métadonnées sont absentes. Une structure présente mais invalide reste une erreur. L'adapter ImageDisk accepte uniquement IMD avec les métadonnées de structure, comme le C#.

Le raccordement aux services globaux de lecture/conversion et à leur composition reste à porter. Ces adapters ne constituent pas une composition complète par défaut.
