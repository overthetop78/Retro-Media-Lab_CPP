# Couverture du contrat de préparation runtime

Rapprochement du contrat C# `IMediaRuntimePreparation` et du service `MediaRuntimePreparationService` avec les déclarations et implémentations C++ actuelles. Le service monolithique est réparti par domaine. Ce tableau établit la couverture des membres ; il ne remplace pas les validations fonctionnelles consignées dans les blocs de portage correspondants.

| Membres C# | API C++ | Implémentation |
|---|---|---|
| CreateBlankDiskAsync, CreateHardDiskAsync | MediaPreparation.h | Preparation/Disks.cpp |
| OpenDiskAsync | RuntimeDisk::openAsync | Runtime/Disk.cpp et les états sectoriels, flux et MFM |
| CreateDriveMechanics, AudioAvailable | MediaDriveService::create, setAudioAvailable | Drives/Service.cpp |
| ReadTapeAsync, WriteTapeAsync, TapeRecordingExtensions | TapePreparation.h | Preparation/Tape.cpp |
| MechanicalSoundProfiles, MechanicalSoundProfileDisplayName | MechanicalSoundService::profiles/profileDisplayName | Drives/Profiles.cpp, Audio/Mechanical/Service.cpp |
| RenderMechanicalSound, MechanicalSoundDurationSeconds | MechanicalSoundService::render/duration | Audio/Mechanical/Service.cpp et RecordedRenderer.cpp |
| ProtectMechanicalAudioMix | protectMechanicalAudioMix | Audio/Mechanical/Mix.cpp |
| FileExtensions | mediaFileExtensions | Preparation/Catalog.cpp |
| RawOpticalSectorSize, AudioSectorSize, OpticalSubchannelSize | rawOpticalSectorSize, audioOpticalSectorSize, opticalSubchannelSize | Preparation/Catalog.cpp |
| ConvertScpAsync | convertScpAsync | Preparation/ScpConversion.cpp |
| ReadCueSheet, IsSingleBinaryRawAudioCue, WithSingleCueFile, WriteCueSheetAsync | CueDocument::parse/isSingleBinaryRawAudio/withSingleFile/write | Preparation/CueDocument.cpp |
| PrepareCueWaveFilesAsync | prepareCueWaveFilesAsync | Preparation/CueWaveFiles.cpp |

L'ouverture possède son résultat et conserve la mécanique empruntée lorsqu'elle est fournie. Les services audio livrent des échantillons ; la sortie hôte appartient au consommateur. Le CUE est un document opaque qui n'expose pas MediaFileSystems.

`Composition/Runtime.h` assemble les registres, lecture/écriture avec le même service mécanique, reconstruction, conversions, exploration et visualisation. Les préparations physiques réutilisent ces services ou leurs fonctions de domaine. `Composition/Runtime.cpp` conserve l'instance partagée ; les façades publiques conservent leur durée de vie sans exporter les types des bibliothèques adjacentes.
