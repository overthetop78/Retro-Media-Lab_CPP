# Validation Macintosh HFS — 15 novembre 2026

Le banc compile 22 fichiers C# originaux, dont les sept sources HFS, les véritables primitives Macintosh et les modèles communs. Seule l’interface IFileSystemReader est réduite aux quatre membres sectoriels afin d’éviter les adaptateurs de documents hors périmètre ; les algorithmes C# restent inchangés. Les fixtures binaires sont générées une fois et transmises identiquement aux deux implémentations.

Résultat Debug : **89 comparaisons, zéro différence**, plus **deux contrôles d’annulation** (read et canRead). Les métadonnées du volume, capacité et espace libre, dates, arborescence, noms/tri, tailles, références, validité et SHA-256 des contenus extraits sont comparés. Tous les avertissements sont comparés avec leurs paramètres et leur ordre aux chaînes originales C#, par rendu explicite des diagnostics dans le banc uniquement.

Les cas couvrent les MDB absents/courts/signatures invalides, le minimum de 162 octets, taille de secteur et taille d’allocation, espace libre impossible, catalogue absent/court et longueur 0–1025, tailles de nœud et repli, compte/table/offsets/clés/noms/records invalides, types Finder, noms MacRoman, identifiants parents et cycles, ordres stables, références élevées, repli ressources, fichiers vides, extents multiples, secteurs absents/courts et longueurs jusqu’à UInt32.MaxValue. Une allocation élevée vérifie aussi le débordement arithmétique signé reproduit explicitement sans comportement indéfini C++.

Un défaut réel du helper commun de type Finder a été trouvé pendant cette préparation : les types composés uniquement de tabulations/CR/LF étaient conservés au lieu de sélectionner le descriptif par défaut. Il a été corrigé dans Macintosh/Primitives par l’agent responsable. Cinq fixtures nouvelles vérifient les espaces ASCII après retrait des NUL/espaces et la conservation d’un type mixte A+tabulation. Aucun autre défaut de production n’a été révélé. Deux corrections concernaient seulement le banc : initialiser l’environnement MSVC avant CMake et expliciter la conversion QJsonValueRef vers QJsonArray pour sa comparaison.

La compilation de production est regroupée par le parent avec les helpers communs ; elle réussit sans avertissement. Les six sources internes sont portées ; MacHfsFileSystemReader reste partiel uniquement pour CatalogFormatIds et le raccordement au registre, sa lecture étant entièrement couverte ici. Aucun writer ni lecture de l’arbre d’extents supplémentaires n’existent dans les sources concernées ; ces fonctions ne sont pas ajoutées artificiellement.

Empreinte du manifeste : `adb9f3545ffa8d67b06ee59fd0d1b5a0d754e9192e68957d2d3044571340ba35`.

| Source C# | SHA-256 |
|---|---|
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/Macintosh/MacFileSystemPrimitives.cs` | `0b8168ea4df288ab9cea6bf0dd0a43958e2b476e4c0bf411db9666d03bc933f2` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/Macintosh/MacFileSystemTime.cs` | `162df6d51709f1187a869a457452e03619fddaceff522a8b72611c4ff6ac7689` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/Macintosh/MacFileSystemExceptions.cs` | `50a932c0e466281c875897573f5c13307c4f8e833e4665bf331ca9400a0df94f` |
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
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/Macintosh/Hfs/MacHfsCatalogReader.cs` | `e7e09c5dc611dccf77c22bd57396ad8a27eea6475126c19cf999df886b299e35` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/Macintosh/Hfs/MacHfsCatalogRecord.cs` | `c85ebdc50346d7c678ebe5279264bfb816b54fae508c04b644bd80cc9585f2b9` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/Macintosh/Hfs/MacHfsDirectoryBuilder.cs` | `f21bcab1ac2a706714a2b30b144e48b6eb728896f039045fcfcf8a81bcd0317e` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/Macintosh/Hfs/MacHfsExtentReader.cs` | `57df0f387f0961f11c6e38d5bdd46af3ab5de8a536271ef3526c050625dec74c` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/Macintosh/Hfs/MacHfsExtentResult.cs` | `7e9ef894c5921aa84fc458ebb55b058f7c23e1f354ec89fa681f823c1164d187` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/Macintosh/Hfs/MacHfsFileSystemLayout.cs` | `6473aaf51c2e4ac1c098c785b106a279450a67d905449c739b4bf19f2a8f8c34` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/Macintosh/Hfs/MacHfsFileSystemReader.cs` | `ba6853243f926b5ca814d5a4d0c1aa8335085fc998e91d72a706e45ed08ad347` |

Les 29 catalogues HFS ont été relus et corrigés (168 messages non anglais), avec contrôle des paramètres, des invariants HFS/Macintosh et de la copie en-US. Le contrôle de littéraux a aussi fait nommer le décalage de deux entrées de la table des offsets, sans changement de comportement ni relance des 89 cas inchangés. L’inventaire des sept sources HFS est conservé dans `macintosh-hfs-inventory.json`.

Après fusion de l’inventaire par le parent et terminaison des processus, le banc temporaire `build/verification-mac-hfs/` et tous ses artefacts ont été supprimés avec vérification du chemin absolu. Aucun test n’a été ajouté au workflow GitHub.
