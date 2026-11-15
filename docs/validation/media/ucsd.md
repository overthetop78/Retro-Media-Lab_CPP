# Validation UCSD p-System — 15 novembre 2026

Le banc compile **28 fichiers C# originaux**, dont les **16 sources UCSD**, leurs modèles et contrats communs. Seule l’interface IFileSystemReader est réduite aux quatre membres sectoriels utiles pour isoler les adaptateurs de documents hors périmètre ; aucun algorithme original n’est recopié. Les mêmes fixtures binaires sont transmises aux implémentations C# et C++.

Résultat Debug : **163 comparaisons, zéro différence**, plus **deux contrôles d’annulation** (read, canRead). Les métadonnées du volume, capacité/espace libre, dates, noms/tri, types, tailles, références de stockage, validité et SHA-256 des contenus sont comparés. Les diagnostics sont rendus dans le banc avec leurs arguments et leur ordre afin de comparer les véritables messages de la source, y compris les erreurs d’en-tête.

Les cas couvrent les deux ordres des octets ; répertoires courts/longs ; blocs absents/courts/trop longs ; entrée traversant une frontière de blocs ; limites des noms et caractères ASCII invalides ; dates nulles, impossibles et pivots 69/70/99/100/127 ; 16 types masqués et bits supérieurs ; dernier bloc utile 0/1/511/512/513/65535 ; plages vides/inversées/hors volume/système ; chevauchements ; trous de données sans décalage ; tri stable ; formats en casse différente, étranger ou avec s long Unicode ; différence volontaire entre CanRead et Read pour un format/géométrie non déclaré.

Aucun défaut de production UCSD n’a été révélé par ces comparaisons. La compilation de production groupée du parent réussit sans avertissement. Les 29 catalogues ont été relus conjointement ; types non typés, octets/quantités, invariants UCSD p-System et intervalle à borne exclusive ont été corrigés, sans génération Argos répétée. Aucun ancien test Apple/HFS n’a été rejoué.

Les 15 composants sont portés ; le Reader reste partiel uniquement pour l’exposition de CatalogFormatIds au registre. Cette validation ne couvre pas de supports physiques et n’ajoute pas de writer absent du C#.

Empreinte du manifeste source : `83a00044a3744723df9d1e11c2ef01ac1bb888b2017344f01656b4279a2f4f01`.

| Source C# | SHA-256 |
|---|---|
| `src/RetroMediaLab.MediaFileSystems/Constants/MediaImageFormatIds.cs` | `cec12745df0195f8517bdca4a579eedac7a88f800aab15b01341c02772a84b56` |
| `src/RetroMediaLab.MediaFileSystems/Definitions/FileSystemIds.cs` | `c03b41106f6376425b0f33c5461086bc407c9382816a8c2a8d317b10fc8eff90` |
| `src/RetroMediaLab.MediaFileSystems/Definitions/FileSystemDisplayNames.cs` | `1b15d63ae224a8a3f5f0341c4d1420186be2d7363bf9b25c67ac9a7a8494a7db` |
| `src/RetroMediaLab.MediaFileSystems/Interfaces/IMediaSectorImage.cs` | `7ce1f0769b0ebdd5677a9ce2a2827f7cacd33d5ebbdc14d12f25a1ee53e5bc2e` |
| `src/RetroMediaLab.MediaFileSystems/Interfaces/IMediaSectorBlock.cs` | `efb3a472042b0a21f93f1dc194a996582a337d00019b9c825826e7124e4b2894` |
| `src/RetroMediaLab.MediaFileSystems/Interfaces/Exploration/IFileSystemEntryView.cs` | `b8e24561e08fe07dbe3fe972c1f68e14eed005faf5ba582bf8df7afaeac2e245` |
| `src/RetroMediaLab.MediaFileSystems/Interfaces/Exploration/IFileSystemVolumeView.cs` | `6105e2c7880d9bba417f09e7150b202ad4a254d5182d038a19dd3793646197ab` |
| `src/RetroMediaLab.MediaFileSystems/Contracts/FileSystemEntry.cs` | `c903d1e07242d561c878c6bab265a38b0f2463b920ce80ee755cd33e40e127c6` |
| `src/RetroMediaLab.MediaFileSystems/Contracts/FileSystemEntryAnalysis.cs` | `5296a241dd5463edbff1941d81f3a362f6680fe9c89b5723794f42190b479ed7` |
| `src/RetroMediaLab.MediaFileSystems/Contracts/FileSystemVolume.cs` | `279d532aea65c9b3551d48fa8e26219298159822dfde99ebd164048b6b92c94e` |
| `src/RetroMediaLab.MediaFileSystems/Enums/FileSystemEntryKind.cs` | `3568e3af543daeadd3385d1b887e6885d861a75f3b0961755521f08cf6163663` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Amiga/AmigaDosVariant.cs` | `bd165f4f35670b247570d16226d4c97ec144bbba875323bda076a729af6c9f0b` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdBlockReader.cs` | `bc2009b572d3e4481fa21ad5b5e5d7813fbb63fe0b41b7f8a54e2c8da0bff669` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdBlockReadResult.cs` | `9fa2b663c5b00b703604c8457dd737a5b3500d8da25f91400a9ea1c552e68bd3` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdByteOrder.cs` | `9b6c4bb1ff966b4b041d5519417c1f01d8db18b84e7a2c2a8749ee4bb0354cc2` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdDate.cs` | `06015b08f66b580e240b91b47b504d6867aa9a251774d578d31b0ea8bd00e254` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdDirectoryEntriesResult.cs` | `02814de3de805df5fa20a845c1b068949e9fefdf4d76b0304dba6d677bb9085f` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdDirectoryEntryReader.cs` | `9c3356f1977b6b97339a015ad94fe085d0f418f224110175817308ef5096009b` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdDirectoryHeader.cs` | `e84d952dc4966a6c8094541bd53067975200d6930cef96848f346200131f61fb` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdDirectoryHeaderReader.cs` | `260810199fffb631f0f16ca4274d5b0b7555a396415b8b65f2eca10320b09ba8` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdFileContent.cs` | `c52d310f6495c759b7a4107dc7bb8e8d28d43a959dc49bd05ee8fd1855e11722` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdFileContentReader.cs` | `501a16854fda86ce05d2f082199b5235aae03fa3841ada562dd32dafc2249f00` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdFileKind.cs` | `c0afde6758049f01d5ddb5657666efcb9323ab5ad183ebd98b9036d112568b89` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdFileSystemExceptions.cs` | `27da3b18b3f96243c89f353af734e26344da7a1440d70abd10d6b2cbfc84ead3` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdFileSystemLayout.cs` | `66f161d3237f77ba2e2ced2908736675b7d9216999ac715d77322d29854134a4` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdFileSystemReader.cs` | `be3964d5e5ee9a702f0bfe20b1312e66b633d7773adb02d8b4d02a7eec966d40` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdName.cs` | `b92f2d1527dcea3e2ab504add4fd1b7d2179b65773ba2d9f5e0f17a41afad368` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Ucsd/UcsdPrimitives.cs` | `360bd6183aff90c31c16374180b6a91e8c23b8da5ca4b46880a5933c5b94d6bd` |

Après fusion parent de l’inventaire et terminaison des processus, le banc `build/verification-ucsd/` et ses artefacts ont été supprimés avec vérification du chemin absolu borné. Aucun test ajouté au workflow GitHub.
