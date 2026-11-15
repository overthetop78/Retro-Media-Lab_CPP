# Représentations par blocs et mécanique des sources

Référence C# locale en lecture seule, consultée le 15 novembre 2026.

Le modèle valide les coordonnées signées, la taille des blocs, la géométrie CHS explicite et les bornes des sources. Les plages sont immuables, triées et non chevauchantes. Stored lit la source ; Zero et Unallocated produisent explicitement des zéros ; Unavailable et les trous échouent, sans remplissage inventé.

La recherche de plage est binaire. Les additions/multiplications checked de C# restent contrôlées. La fusion sans checked préserve le comportement arithmétique signé par calcul non signé puis réinterprétation, sans comportement indéfini C++.

Le contrat de bloc est celui de MFS : ses erreurs de lecture sont typées et traduisibles dans ce contrat, afin que l'explorateur puisse distinguer corruption, E/S et annulation sans dépendre d'Engine. Engine possède l'implémentation, les sources et la mécanique.

La vue RepresentationInfo destinée à App ne référence aucun type MFS. L'implémentation BlockImage fournit simultanément les propriétés de cette vue et du contrat de lecture MFS, avec une seule donnée possédée.

La source mécanique générique accepte deux callbacks de position/contiguïté. La liaison des blocs les construit depuis la géométrie ; les autres représentations pourront réutiliser la même source. Le bloc ne possède ni horloge ni sortie audio et ne réinitialise pas le lecteur partagé.

Les accès sont fermés via RAII, y compris si BeginRead/BeginWrite échoue après initialisation partielle. Une erreur EndAccess sur le chemin normal se propage ; pendant le déroulement d'une autre exception, le nettoyage est tenté sans remplacer l'erreur d'origine. C'est une protection explicite répondant à la règle projet de libération sur exception.

Une lecture asynchrone conserve son état partagé et ses sources jusqu'à son exécution ; le tampon destination reste possédé par l'appelant jusqu'à la fin du future, conformément au contrat MFS. Les copies réutilisent des fragments de 64 Kio et la sortie existante du writer atomique.

Seule la partie blocs de MediaDriveBindingFunctions est portée ici ; ses raccordements secteur/optique et le service concret de simulation mécanique restent à porter dans les blocs concernés.

## Provenance

- `Contracts/MediaDataRange.cs` : `5a865df81604b8344b50a9af212fc0a77da00abdcfe863e55bced7c04e1d6c12`
- `Enums/MediaDataRangeKind.cs` : `55ea61300aa32dcff9db36c82484185f1111f8479740cebdbf9010d1a0ba5afe`
- `Contracts/HardDiskGeometry.cs` : `0d4cebdab82caa771aa28b03796ea40afe5da80b8414c1ab6926a16b8642b901`
- `Images/Models/Blocks/BlockMediaImageRepresentation.cs` : `4024cf48fc9949a5fa90b9fd597c002851275539a519bd592a9c4ab971afac01`
- `Images/Reading/Blocks/BlockMediaDataReader.cs` : `d9db120adc9a19706f2eb3af86679038680077c8908709018e6b899148359b57`
- `Contracts/MediaDrivePosition.cs` : `25d153ba087746a092f265ee6394d7e3647a368cb6f23c08963db3a52372d37b`
- `Contracts/MediaDriveState.cs` : `035a516e1844b7607e61552f36c2b5674a28fc8f4827d8eaddc5b6e3ba24cf6d`
- `Interfaces/IMediaDriveMechanics.cs` : `4adf173be5020e028b74b552dd4e0030e6f303efeece4cc7e4a7650de2ee98b9`
- `Images/Reading/Sources/MediaDriveRandomAccessData.cs` : `1aa6c86f2fa3e525bb11d5689eaa42e86aea71ceb74544bd654bc572db130d48`
- `Enums/MediaRepresentationKind.cs` : `52504046ff1a920220325b5b5abff024ad20643001aa35bcf19d08a51bb25dca`
- `Interfaces/IMediaImageRepresentation.cs` : `64f81f39af45540a8e33da8ab83ad073a9333f0f398f6bc0aee96c786853a3d0`
- `Functions/MediaDriveBindingFunctions.cs` : `24c14b0aae4e33198f9aa94a7a483498110d04b1ae698086355a6975b79e4114`

## Référence Microsoft consultée pour l’édition de liens

Le dump COFF a identifié la section abstraite RepresentationInfo (`selection 6`, pick largest) dans l'objet signalé par LNK1163. Aucun ILK résiduel n'était présent : l'hypothèse d'un simple ancien état incrémental n'est pas confirmée.

Microsoft réserve `novtable` aux interfaces pures non instanciées. L'attribut retire l'initialisation vfptr de leurs constructeurs/destructeurs et permet de retirer leur table abstraite. Le projet applique cet attribut uniquement sous MSVC ; la déclaration reste standard sur les autres compilateurs.

- [Microsoft — novtable](https://learn.microsoft.com/en-us/cpp/cpp/novtable?view=msvc-170)
- [Microsoft — édition de liens incrémentale](https://learn.microsoft.com/en-us/cpp/build/reference/incremental-link-incrementally?view=msvc-170)

Ces documents décrivent le mécanisme, pas une cause garantie de LNK1163 ; l'effet concret de la correction doit être confirmé par la compilation du projet.
