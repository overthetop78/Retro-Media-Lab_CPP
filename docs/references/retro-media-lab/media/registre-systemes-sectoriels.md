# Registre sectoriel et composition des lecteurs — référence C#

Le catalogue par défaut C# contient 21 lecteurs, dans l'ordre ci-dessous. Le port C++ doit conserver cet ordre pour les cas ambigus ; un catalogue réduit aux lecteurs déjà disponibles ne constitue pas le port complet du catalogue par défaut.

1. `Amiga.AmigaDosFileSystemReader`
2. `Amiga.FlatArchive.AmigaFlatResourceArchiveReader`
3. `Acorn.Adfs.AcornAdfsFileSystemReader`
4. `Acorn.BbcDfs.BbcDfsFileSystemReader`
5. `Coherent.CoherentFileSystemReader`
6. `Dec.Rt11.Rt11FileSystemReader`
7. `Ucsd.UcsdFileSystemReader`
8. `Apple.Dos.AppleDosFileSystemReader`
9. `Apple.ProDos.ProDosFileSystemReader`
10. `Apple.Macintosh.Mfs.MacMfsFileSystemReader`
11. `Apple.Macintosh.Hfs.MacHfsFileSystemReader`
12. `Apple.Lisa.LisaFileSystemReader`
13. `Cpm.AmstradCpmFileSystemReader`
14. `Cpm.CpmFileSystemReader`
15. `Commodore.Dos.CommodoreDosFileSystemReader`
16. `Fat12.Fat12FileSystemReader`
17. `Atari.SpartaDos.SpartaDosFileSystemReader`
18. `Atari.Dos.AtariDosFileSystemReader`
19. `Atari.KFile.AtariKFileFileSystemReader`
20. `Atari.ClkGraphicsLibrary.AtariClkGraphicsLibraryFileSystemReader`
21. `Atari.BootDisk.AtariBootDiskFileSystemReader`

## Contrats à préserver

- Les lecteurs exposent un identifiant et les identifiants centraux de formats acceptés. Les fonctions C++ de lecture déjà présentes ne remplacent pas encore ce contrat de découverte.
- Le registre copie l'ordre d'entrée, refuse les lecteurs nuls, les identifiants vides et les doublons sans casse. Les index par lecteur et par format ne modifient pas l'ordre des candidats.
- `ReadAll` sonde tous les lecteurs ; `ReadCandidates` privilégie un identifiant de lecteur avant un identifiant de format, retourne un rapport vide pour un identifiant inconnu, et utilise tous les lecteurs pour une sélection absente.
- La sonde `CanRead` précède la tentative de lecture. Les corruptions reconnues pendant `Read` sont rapportées séparément ; les erreurs de sonde et les annulations ne doivent pas être absorbées comme absence de format.
- Les interprétations distinctes utilisent les formats indexés quand ils existent, sinon l'ensemble des lecteurs. La première image conserve toutes ses correspondances ; les images suivantes n'en conservent que la première avant déduplication.
- L'identité contient la famille avant le premier point, le nom du volume et les chemins/types/tailles des entrées triées récursivement sans casse. Séparateurs : NUL entre champs, U+001F entre entrées, `/` entre segments. Préserver les noms Unicode et les règles de comparaison plutôt que remplacer celles-ci par une normalisation arbitraire.
- Le contrat C# adaptant un document générique exige une représentation sectorielle et un volume couvrant l'image entière. Cette adaptation doit être traitée avec les contrats de documents, sans la confondre avec la simple lecture sectorielle.

## État lors du relevé

DOS, ProDOS, MFS et HFS possèdent leurs fonctions de lecture C++ vérifiées ; leurs métadonnées de découverte restent suivies comme partielles dans l'inventaire. Lisa vient d'être vérifié ; UCSD et CP/M sont en cours dans des périmètres indépendants. RT-11 et les lecteurs Atari restent à porter avant la composition complète des 21 lecteurs. La fonction Engine `exploreSectorImage` reste limitée à FAT12 et doit ensuite être raccordée au registre complet.

## Empreintes des sources consultées

| Source C# | SHA-256 |
|---|---|
| `Interfaces/IFileSystemReader.cs` | `e46845477d3d1325107619dde1de8d51b7d5fa7c29b4bdbb0d99f1a32a1b2dd9` |
| `Exploration/SectorFileSystemRegistry.cs` | `3bcc8972c33ddc1bdb9d4b5ebcd926b61e90580262a468acd8891e1ad3dfbe8b` |
| `Exploration/FileSystemReaderCatalog.cs` | `0e8eef816a343f9d6e2af394bf332b0a5123cd4b8aa2f85038f9e96f912def7a` |
| `Exploration/Interpretation/FileSystemInterpretationIdentity.cs` | `73b0318c74de6fb9f510f7e4bcc696585df3d3280e340806fb5df0cb25b8db63` |
| `Contracts/FileSystemReadReport.cs` | `2912e5ac14c7bcb6812366677895ebf2a637b399fae798bafd77df8accc9b8b7` |
| `Contracts/SectorFileSystemCandidate.cs` | `732ae02796226f31f9eab057bb380f7ebf6796e98c7ebd3077f2e4a3de33f416` |
