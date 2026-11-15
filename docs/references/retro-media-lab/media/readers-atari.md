# Readers Atari et BPB ST

Sources C# consultées en lecture seule. Les readers communs ST/MSA/ATR/ATX réutilisent les backends natifs et la fabrique sectorielle. Le contexte conserve les lectures bornées et le snapshot ; aucune logique App n’est ajoutée.

ST reprend la détection propre au C# : BPB prioritaire, total 32 bits plafonné à ushort.MaxValue lorsqu’il remplace le champ 16 bits, au plus 36 secteurs par piste et 2 têtes. À défaut, secteurs 9/10/11/18 dans cet ordre, puis têtes 2/1, cylindres entre 35 et 90. Cette politique n’est pas remplacée par la politique générique FAT, qui accepte d’autres limites. Les offsets BPB existants sont déplacés dans le contrat public MFS et réutilisés par alias en interne, sans duplication ni accès Engine aux en-têtes privés de MFS.

Le reader ST initialise l’intégrité logique des blocs complets comme le constructeur SectorBlock C# ; cela ne prétend pas prouver un CRC physique. Sa source reste partagée et les blocs sont matérialisés lors des lectures, sans allouer à l’avance une collection proportionnelle à tous les secteurs.

Les capacités ST/MSA utilisent le préfixe atarist. ; ATR accepte les quatre formats déclarés et le préfixe atari.atr. ; ATX accepte son identifiant exact. Les signatures déclarées et sondes MSA/ATX sont reprises avec leur ordre d’octets. ATR valide le conteneur et ne décline que les erreurs de données correspondantes ; les erreurs d’E/S et l’annulation ne sont pas masquées. Comme en C#, les sondes ne sont pas limitées par l’extension seule.

Les writers Atari, les erreurs qui leur sont propres et la composition globale de reconnaissance restent séparément à porter/raccorder. AtariStExceptions est donc explicitement partiel dans l’inventaire.

| Source | SHA-256 vérifié |
| --- | --- |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/Atr/AtrReader.cs` | `712bec9067981aa5701bec824aa4aec3b16e6aa0012cf19abb0eca31cc94558f` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/Atx/AtxReader.cs` | `0b2db626c753ed88e594903adcb2366cf2b14896f4791902d4599a25a541872d` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/Msa/MsaReader.cs` | `4ea1867be328a98b0bb6f39d0faf2f083f3fa0890990ec91a776a9d712fd71c3` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/St/AtariStExceptions.cs` | `bc96045e40c9ede99f28012d3601d35b0f403bb9a7165b1c17af0e4d00930301` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/St/AtariStGeometryDetector.cs` | `8e8c12286fb87e534f2fe1e620cc06f8a4129a46d3cdc05c549b44a64513cba9` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/St/AtariStReader.cs` | `606141a7d9fff0cc74829868e12de322373eb2110c65ebae6b54ff8c6de4a114` |
| `src/RetroMediaLab.MediaFileSystems/FileSystems/Fat12/FatBootSectorLayout.cs` | `e517044ab38a1c48872758495f1e0a1cb4e74d497a7c279bf90dd235a636b730` |
