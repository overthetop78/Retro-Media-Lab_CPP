# Validation Atari SpartaDOS — 15 novembre 2026

Le banc compile **27 fichiers C# originaux**, dont les **12 sources SpartaDOS**, les primitives little-endian 16/24 bits et les modèles communs. Seule l’interface IFileSystemReader est réduite à ses quatre membres sectoriels pour isoler les adaptateurs de documents hors périmètre ; aucun algorithme source n’est réécrit. Les fixtures binaires sont générées une fois et transmises identiquement aux deux implémentations.

Résultat Debug : **177 comparaisons, zéro différence**, plus **deux contrôles d’annulation** (canRead et read). Sont comparés nom/identifiants/nom affichable du volume, capacité/espace libre, attributs, bootable, hiérarchie et ordre, noms/dates/création, flags/validités, nativeTypeId, occupiedSize, attributs sparse et SHA-256 de tous les contenus. Les erreurs structurées sont rendues avec les formulations et arguments exacts du C# dans le banc uniquement.

Les fixtures couvrent 128/256 octets et boot court de 128 sur géométrie 256 ; tailles boot 42/43 ; signatures/versions 1.x/2.x ; total et compte libre ; plages bitmap/root ; verrouillage et démarrage ; références de cartes nulles/hors plage ; cartes courtes, absentes, backlinks incohérents, boucles, continuation, chaîne trop courte ; validation de next même après le dernier pointeur utile ; zones creuses ; données absentes/courtes/longues ; fichiers vides sans validation inutile de leur map ; longueurs aux frontières de secteurs et hors capacité ; flags combinés/inutilisés/supprimés ; sous-répertoires, cycles et réutilisation entre frères ; champs date/heure nuls, bissextiles et impossibles ; noms ASCII/haut bit/NUL/extension seule. Un format de conteneur étranger vérifie la détection sur en-tête, sans filtre artificiel.

Aucun défaut de production SpartaDOS n’a été mis en évidence. La référence C# et les compilations groupées de production réussissent sans avertissement. Les 29 catalogues ont été relus : 203 messages, 125 corrections ciblées, préservation des paramètres/SpartaDOS et normalisation de la plage 1..{1}. Les boucles de maps/répertoires perdues dans plusieurs traductions, les quantités d’octets, la distinction prédécesseur observé/attendu et le chinois traditionnel ont été corrigés. Aucun ancien banc AtariDOS/RT-11 n’a été rejoué.

Les onze composants sont portés. SpartaDosFileSystemReader reste partiel uniquement pour CatalogFormatIds et le raccordement au registre, sa lecture et sa détection étant portées. Le contenu du bitmap n’est pas interprété dans la source et le port ne prétend pas le vérifier : l’espace libre reste celui annoncé par l’en-tête. Aucun writer absent du C# n’est ajouté. Cette validation n’utilise pas de support physique.

Empreinte du manifeste source : `65849595cec27962b2d5db6f5bdfd4617e8ef53506bddcdb207bfa6384c40e63`.

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
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Atari/SpartaDos/SpartaDosDirectoryFlags.cs` | `800385b48284f2448f2f2aab5385b06a7066cfeb591aff3cc9e2874889b1b1f2` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Atari/SpartaDos/SpartaDosDirectoryReader.cs` | `11e49e09995d5a5574607b95aaf8ecf9daa0974a80a01cc1287ba1f7a1d28580` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Atari/SpartaDos/SpartaDosDiskHeader.cs` | `af6d6cde032c6bb48efc1c08d219626ee3ed93b0146d86627dba026d373a9e12` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Atari/SpartaDos/SpartaDosDiskReader.cs` | `822c7a3b41365572c7ff69fe51c221118815e3dc58cdc25c6c15c5cb5e33d423` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Atari/SpartaDos/SpartaDosFileData.cs` | `eff2d1d79ae47f08f5eb2c25beb0cb55ac98839812e16336e2b7a1780f501c40` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Atari/SpartaDos/SpartaDosFileReader.cs` | `caff59a70d51f78b3c80db8230bae1978e8049cc75c2b0f3d0384246907f4955` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Atari/SpartaDos/SpartaDosFileSystemExceptions.cs` | `1c8e5960ffc953ecfe62b3161c13b8a12fdc2a30106dbeef7e6b6167599b3f24` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Atari/SpartaDos/SpartaDosFileSystemLayout.cs` | `b241ce39e2d97bad90ec2803f273b4c133d100e4e829902267a6b8b359255da0` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Atari/SpartaDos/SpartaDosFileSystemReader.cs` | `d0a7879bb968a9be14dcec00a6468e9b029e7b8bec82e1612ce93bc8d18940e2` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Atari/SpartaDos/SpartaDosNameCodec.cs` | `a0374cbb46c6f6e8723d17037483dc0c864856cd7d27d6d9476adfe11196e2fb` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Atari/SpartaDos/SpartaDosSectorAllocation.cs` | `b0e424d15346afaa52ac8608f716b8d342c041c3e49c3ef7b3e8921f9ba28e9b` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Atari/SpartaDos/SpartaDosSectorMapReader.cs` | `414817934d50363461fc09ee1a292523c792b4ec9a5f4ae6029f857661d1463a` |
| `src/RetroMediaLab.MediaFileSystems/Primitives/LittleEndianUInt16.cs` | `a8aecbb3337e4b781f43811e784bb6267914dc1c07d6d9ff87720760265df891` |
| `src/RetroMediaLab.MediaFileSystems/Primitives/LittleEndianUInt24.cs` | `0a37a1c22549bd12486f365c6f206b48a8d56d410ed55c31d762f314610b1836` |
| `src/RetroMediaLab.MediaFileSystems/Constants/BitConstants.cs` | `2314b3596bebb72f7548d81f7ed6bb3919783cc2b68fa83c63876df4c260aa47` |

Après fusion parent de l’inventaire et terminaison des processus, le banc `build/verification-spartados/` et ses artefacts ont été supprimés avec vérification de son chemin absolu borné. Aucun test ajouté au workflow GitHub.
