# Validation ProDOS/SOS — 15 novembre 2026

Le banc compile 37 fichiers C# originaux, dont les 19 sources ProDOS. Aucun algorithme de référence n’est réécrit. Seul le contrat IFileSystemReader est réduit aux quatre membres sectoriels utiles pour isoler ces véritables algorithmes des adaptateurs de documents non concernés. Les plans d’écriture et les modèles restent les sources originales.

Résultat Debug : **150 comparaisons ciblées, zéro différence**, obtenues par une première campagne de 147 cas puis 3 cas additionnels sans rejouer la première. S’ajoutent trois contrôles d’annulation (read, canRead, write) et un contrôle de conservation de l’horodatage par défaut explicite. Les compilations de production sont regroupées par le parent et réussissent sans avertissement C++.

La comparaison couvre noms invalides et limites, années/pivots/jours bissextiles, fichiers vides et limites seedling/sapling/tree, création ProDOS 140/800, sous-répertoires et débordements de répertoires, attributs, contenus absents, longueurs excessives, limites de profondeur, cycles/réutilisations, index creux, secteurs manquants/courts, bitmap absent, volume déclaré trop grand, types connus/inconnus et sélection SOS. Les octets écrits sont comparés par SHA-256 ; métadonnées, hiérarchie, contenus extraits et arguments/ordre des avertissements sont comparés. Les 3 cas additionnels contrôlent aussi chaque adresse de bloc d’une géométrie zonée ainsi que le choix SOS en casse ASCII, sans assimiler un s long Unicode à un s ASCII.

L’heure UtcNow du writer C# n’est pas injectable. Les quatre octets de création de son en-tête sont donc normalisés des deux côtés avant comparaison, uniquement sur la racine ou un en-tête de sous-répertoire validé par sa structure. Les dates des entrées restent intégralement comparées. Le contrôle natif séparé vérifie l’horodatage déterministe C++. Les libellés français de types de la source sont comparés par signification technique, les traductions étant relues séparément.

Les 29 catalogues ont été relus conjointement avec le parent ; paramètres et invariants ProDOS/SOS ont été vérifiés. BASIC et la notation technique ProDOS ${0} ont été retirés des langues et conservés uniquement dans Constants. Le diagnostic dédié de Timestamp invalide est ajouté séparément ; son contrôle ciblé passe après compilation groupée, sans rejouer les comparaisons précédentes.

Aucun défaut d’algorithme de production n’a été mis en évidence par ces comparaisons. Les capacités writer hors plage 7–4096 blocs sont explicitement rejetées, car le writer source ne possède qu’un bitmap et indexe hors limites en dehors de cette plage. Cette validation ne constitue pas une campagne sur des supports physiques ni une preuve de validité de tous les fichiers ProDOS existants.

Empreinte du manifeste source : `35f370e5dce32cff5f0e9002d680ed3c9387c61285ce2cb85cb77427e3803aa5` ; les 37 empreintes correspondent à l’inventaire de référence au moment du test.

| Source C# | SHA-256 |
|---|---|
| `src/RetroMediaLab.MediaFileSystems/Constants/DiskGeometryConstants.cs` | `9a540154f199f0f5177aab6e5ad68eb5285f02b318897c11db7cf791871c0c05` |
| `src/RetroMediaLab.MediaFileSystems/Constants/MediaImageFormatIds.cs` | `cec12745df0195f8517bdca4a579eedac7a88f800aab15b01341c02772a84b56` |
| `src/RetroMediaLab.MediaFileSystems/Contracts/FileSystemEntry.cs` | `c903d1e07242d561c878c6bab265a38b0f2463b920ce80ee755cd33e40e127c6` |
| `src/RetroMediaLab.MediaFileSystems/Contracts/FileSystemEntryAnalysis.cs` | `5296a241dd5463edbff1941d81f3a362f6680fe9c89b5723794f42190b479ed7` |
| `src/RetroMediaLab.MediaFileSystems/Contracts/FileSystemVolume.cs` | `279d532aea65c9b3551d48fa8e26219298159822dfde99ebd164048b6b92c94e` |
| `src/RetroMediaLab.MediaFileSystems/Contracts/MediaSectorAddress.cs` | `ecd9f739ad2d15a157dcfc3a198ebafc2db0bfc9aaf1386678d2350607c2bada` |
| `src/RetroMediaLab.MediaFileSystems/Contracts/MediaSectorGeometry.cs` | `f242785a41cea760ce75b2385f07a30ee7a755d97d7b91d769d741b2c444e48d` |
| `src/RetroMediaLab.MediaFileSystems/Contracts/MediaSectorWriteBlock.cs` | `313d74915aabe781a95759645d6e8f674512b11b7ddc8c28bd05f7263dddac45` |
| `src/RetroMediaLab.MediaFileSystems/Contracts/MediaSectorWritePlan.cs` | `7fbcadf49aa3758643d0d44eaf4ba129017418387c37867487deb389d0192b89` |
| `src/RetroMediaLab.MediaFileSystems/Definitions/FileSystemIds.cs` | `c03b41106f6376425b0f33c5461086bc407c9382816a8c2a8d317b10fc8eff90` |
| `src/RetroMediaLab.MediaFileSystems/Enums/FileSystemEntryKind.cs` | `3568e3af543daeadd3385d1b887e6885d861a75f3b0961755521f08cf6163663` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosBitmapReader.cs` | `a3f51491aca7cd5d38377749e652050b4f752ca9364dfe763a7c6b0d8b48478f` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosBitmapResult.cs` | `eeffdd142f4a7501aee749cb4bd86097fb88f07b9c06689f86120f8e6f3c5db3` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosDateTime.cs` | `253af4ff797caf8d7d7a41d25dda85866c5dc9e8e409b185132a09d83241b4f0` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosDirectoryReader.cs` | `aab28a8831984d99d520448d49b89c788f085092f049a869225dcec455952b2f` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosDirectoryResult.cs` | `e2821fd6567d0c6dd715cb797adc01e1c723f87d24ef602be757eecc0699445f` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosFileContent.cs` | `a6efb0e27635fe7398466440ac51e235a23b152fa5000e5b51d75a5ae8defeb1` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosFileContentReader.cs` | `ebc1d4534840f2e6d67571d7225e988870f1221c90ff2366738d79f1447af303` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosFileSystemExceptions.cs` | `3206fa5edef945d9727b7babe42528d3fff507b1d00e4d6440c8bb513506e3f9` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosFileSystemLayout.cs` | `705ce324038e023988fee880a33d105aa1137c47b56169226624daed8a071c05` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosFileSystemReader.cs` | `4cadf37b912977d48438b49576a2c828abef6fffe1888e6fb93292a0e5234fe5` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosFileType.cs` | `c80db9f9afe140f3007be69e716f812b20b3da51be468a20d9551d39cebef0b4` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosFileTypeNames.cs` | `6e48615d5bdfe0619706b9c4c33e3f5754855369dc5b2e99bc00a39b23911f32` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosNamePolicy.cs` | `11c13dc2b8d357d3d9ffdc956378a5cb88ddf9249666d1cc541674c128a2a44b` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosPrimitives.cs` | `ffa2c206e9eb766e71c33b7aac5f0daf71daf2785510e0cc35861b0da340b12b` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosStorageType.cs` | `1cafff5c9bb42e3f7a0c77511615f375e73137999ca26835f18ef1264b2109d8` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosVolumeHeaderInfo.cs` | `18cc6f41e764d0bc7e78d22ab23ad3444f87553a7e067d771fdd67d516356aef` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosVolumeHeaderReader.cs` | `15ee0cd48104f8811431b574b39739480b9f0ea1dd37ada28e814af8fedc3a77` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosVolumeWriter.cs` | `a80166b3861ff3356ab6f5e3e007813e33c9c1620d9dbbc1af24f12b22725763` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Apple/ProDos/ProDosVolumeWriterExceptions.cs` | `cd784eaf1ad72a5d9953659fcf31e0697813749680d16e94852fafecdc7990a9` |
| `src/RetroMediaLab.MediaFileSystems/Interfaces/Exploration/IFileSystemEntryView.cs` | `b8e24561e08fe07dbe3fe972c1f68e14eed005faf5ba582bf8df7afaeac2e245` |
| `src/RetroMediaLab.MediaFileSystems/Interfaces/Exploration/IFileSystemVolumeView.cs` | `6105e2c7880d9bba417f09e7150b202ad4a254d5182d038a19dd3793646197ab` |
| `src/RetroMediaLab.MediaFileSystems/Interfaces/IMediaSectorBlock.cs` | `efb3a472042b0a21f93f1dc194a996582a337d00019b9c825826e7124e4b2894` |
| `src/RetroMediaLab.MediaFileSystems/Interfaces/IMediaSectorImage.cs` | `7ce1f0769b0ebdd5677a9ce2a2827f7cacd33d5ebbdc14d12f25a1ee53e5bc2e` |
| `src/RetroMediaLab.MediaFileSystems/Interfaces/Migration/IMigrationNamePolicy.cs` | `df1e275f28a9024cad541e800201d75c84dbf48efe1c8f08ea2d7309a8f98e2e` |
| `src/RetroMediaLab.MediaFileSystems/Migration/MigrationEntry.cs` | `8d266cbc55bdca115a5406fade83bae1abc8b4778b2ffabc411612d287466d14` |
| `src/RetroMediaLab.MediaFileSystems/Migration/MigrationPlan.cs` | `3945a0a3da5324796cc171e3a92d5025d48b80ea9158a058eebc28558e33edd4` |

La lecture fonctionnelle est portée. La propriété C# `CatalogFormatIds` de ProDosFileSystemReader et son raccordement au registre IFileSystemReader ne sont pas exposés dans ce bloc ; cette source reste partielle pour ces seules métadonnées d’enregistrement (18 autres sources portées).

Le banc temporaire `build/verification-prodos/`, ses références compilées, DLL chargées uniquement durant les tests et fichiers générés ont été supprimés après terminaison des processus. Aucun test ajouté au workflow GitHub. L’inventaire détaillé des 19 sources ProDOS est conservé dans `prodos-inventory.json`.
