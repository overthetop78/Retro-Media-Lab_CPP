# Documents, reconnaissance et conteneurs Amstrad/FDS

Référence C# consultée en lecture seule. Les identifiants réels de formats sont amstrad.cpr, amstrad.rom et nintendo.fds ; ils diffèrent des identifiants des writers.

Le contexte C++ conserve les opérations différées et partage les pages, y compris leurs échecs, sans cycle de possession. Les messages conservent clé, arguments et domaine ; les noms virtuels traduisibles restent séparés des noms physiques. Les vues publiques Engine ne publient pas les en-têtes MFS.

CPR conserve les banques éparses, les identifiants cb00–cb31 et leurs tailles réelles. ROM conserve les huit tailles de corpus et les critères de reconnaissance AMSDOS/Dandanator ; une ROM courte est accessible explicitement, sans compléter sa banque. FDS conserve les faces complètes et l’en-tête original lors de l’écriture. Les writers réutilisent la sortie atomique partagée. SupportsTarget reste indépendant du document.

La liaison mécanique du document couvre actuellement les blocs uniquement. Les liaisons secteur/optique, le service concret de simulation mécanique et le raccordement global des autres formats restent à porter.

| Source | SHA-256 vérifié |
| --- | --- |
| `src/RetroMediaLab.MediaEngine/Contracts/MediaImageDocument.cs` | `21c54aba4d16d6dae3e3cb887d14009c071603a2e0e54576931101ee88de3a33` |
| `src/RetroMediaLab.MediaEngine/Contracts/MediaSourceDescriptor.cs` | `1bc9c4ff61da3729305a9bd7b7b4c3095f143559ac41a71dcfd2d491c85b8bd5` |
| `src/RetroMediaLab.MediaEngine/Contracts/MediaVolumeInfo.cs` | `48a937f149f8c17b0f78fbc919b74fcdb2a604154c574812f142997b9cd4085c` |
| `src/RetroMediaLab.MediaEngine/Enums/MediaKind.cs` | `2a0c68ac9ab1bddcd2442f1de8191bc4ded1a6e6c6d46725281e5a7451b116db` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Cartridge/AmstradCpr/AmstradCprReader.cs` | `6a4aed65db8b543483c024c9f449b99571f1b45bf3bc1697fe55526620d212ad` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Cartridge/AmstradCpr/AmstradCprWriter.cs` | `9a6f3d462312215f63b2f685359abf63764d5fce936b5cb1915871a0407ba7fe` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Cartridge/AmstradRom/AmstradRomReader.cs` | `45bcdb8208495e6cf76b497985dca393264be1bd9c4e0f568e0e61b0886d6609` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Cartridge/AmstradRom/AmstradRomWriter.cs` | `8982f78100dea1c55b99391d8f2dfba299008eefdd00e415ba77b020c8c2a5c8` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/FamicomFds/FamicomFdsConstants.cs` | `c2365a0e49e004f2239e1f22dec8992d70f36695d1e1f5d777767c5f6110d163` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/FamicomFds/FamicomFdsReader.cs` | `dc0782e75ac91e9de0c919ba8323ac9f3d38460cc303366f841b0653c6964af8` |
| `src/RetroMediaLab.MediaEngine/Images/Formats/Floppy/FamicomFds/FamicomFdsWriter.cs` | `aa0e9fae2731244826bb6178cf454e47fe48cfc69b08bd9d8c2d8e7b9879714b` |
| `src/RetroMediaLab.MediaEngine/Images/Reading/Recognition/MediaRecognitionContext.cs` | `b2a8ec1b8287b754c1ed171ebef9f3b02b3422c980cae7a16aacbff92df32d43` |
| `src/RetroMediaLab.MediaEngine/Interfaces/Reading/IMediaImageReader.cs` | `ccc37bf7fdfe59b63e7a58d9f3d49c7e6dbadf2a6ac578ad17b6a4ed28db497c` |
| `src/RetroMediaLab.MediaEngine/Interfaces/Writing/IMediaImageWriter.cs` | `9f7bcf5e43651a0d3c300ffe5453e95bca6b1de129baafc86be86234ade50cbf` |
| `src/RetroMediaLab.MediaFileSystems/Functions/MediaMetadataFunctions.cs` | `ff9102641a16ba24e5e8a2389e64192a4d069519fb82354553752f17eb0b7a16` |
