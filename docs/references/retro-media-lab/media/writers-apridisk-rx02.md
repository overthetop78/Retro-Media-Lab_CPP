# Raccordement des writers Apridisk / RX02

Source : `Images/Writing/MediaWritingComposition.cs`, entrées Apridisk et DecRx02, et `Images/Conversion/DecRx02ConversionService.cs`, consultés dans le dépôt C# en lecture seule.

Les fonctions `Apridisk::writerAdapter()` et `Rx02::writerAdapter()` exposent les backends natifs au registre d’écriture commun. Les identifiants `apridisk` et `dec-rx02` reprennent `Constants/MediaImageWriterIds.cs`. Le contrat générique filtre identifiant de format, extension normalisée et représentation sectorielle avant le prédicat du backend.

Apridisk accepte la géométrie Apricot indépendamment du nom du profil source et réétiquette l’image vers Apricot avant l’écriture, comme `WithFormatId` dans la composition C#. Le wrapper garde le propriétaire partagé de l’image ; aucune copie des secteurs n’est nécessaire pour ce changement d’identifiant. RX02 conserve l’exigence du profil RX02 natif et la cible IMG. Les deux sorties passent par l’écriture atomique existante et retournent un unique chemin produit.

La composition globale et les services de conversion complets restent à porter ; exposer ces adapters ne constitue pas leur raccordement à l’application.
