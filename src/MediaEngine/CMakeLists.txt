add_library(RetroMediaLab.MediaEngine SHARED
  src/Runtime/FluxTrackData.cpp
  src/Runtime/Tracks.cpp
  src/Runtime/WinchesterTracks.cpp
  src/Runtime/Disk.cpp
  src/Runtime/Sectors.cpp
  src/Runtime/Persistence.cpp
  src/Runtime/SectorTracks.cpp
  src/Runtime/FormatTrack.cpp
  src/Runtime/Metadata.cpp
  src/Composition/Writing.cpp
  src/Images/WritingService.cpp
  src/Formats/Cartridge/ConsoleWriter.cpp
  src/Preparation/CueDocument.cpp
  src/Preparation/CueWaveFiles.cpp
  src/Drives/Profiles.cpp
  src/Drives/Mechanics.cpp
  src/Drives/Audio.cpp
  src/Drives/Service.cpp
  src/Audio/Mechanical/RecordingDecoder.cpp
  src/Audio/Mechanical/RecordedBank.cpp
  src/Audio/Mechanical/RecordedRenderer.cpp
  src/Audio/Mechanical/RecordedSounds.cpp
  src/Images/ReadingService.cpp
  src/Images/Creation/Blank.cpp
  src/Images/Creation/Catalog.cpp
  src/Images/Creation/Migration.cpp
  src/Formats/Cartridge/Console.cpp
  src/Composition/Recognition.cpp
  src/Formats/Tape/Common/SignalIO.cpp
  src/Formats/Tape/Common/SignalBlocks.cpp
  src/Formats/Tape/Common/BlockIndex.cpp
  src/Audio/Conversion.cpp
  src/Formats/Common/Audio/DecodedPcm.cpp
  src/Formats/Tape/Audio/Reader.cpp
  src/Images/Conversion/Sequential/Service.cpp
  src/Images/Conversion/Sequential/Planning.cpp
  src/Images/Conversion/Sequential/SignalConverter.cpp
  src/Composition/SequentialMedia.cpp
  src/Formats/Optical/BinCue/Sheet.cpp
  src/Formats/Optical/BinCue/SheetWriter.cpp
  src/Models/Sequential/Image.cpp
  src/Formats/Tape/Common/PcmProfile.cpp
  src/Formats/Tape/Common/PcmSamples.cpp
  src/Formats/Tape/Common/PcmSignal.cpp
  src/Formats/Tape/Common/UefSignal.cpp
  src/Formats/Tape/Common/UefDocument.cpp
  src/Models/Sequential/TapeSignal.cpp
  src/Formats/Tape/Common/PcmDocument.cpp
  src/Formats/Tape/Simh/Parser.cpp
    src/Formats/Tape/Simh/Reader.cpp
    src/Formats/Tape/Simh/Writer.cpp
    src/Formats/Tape/Voc/Reader.cpp
    src/Formats/Tape/Voc/Writer.cpp
    src/Formats/Tape/Wav/Reader.cpp
    src/Formats/Tape/Wav/Writer.cpp
    src/Formats/Tape/Raw/Reader.cpp
  src/Formats/Tape/Raw/Writer.cpp
  src/Images/Optical/CompanionAudio.cpp
  src/Images/Optical/DiscSession.cpp
  src/Formats/Optical/WiiU/Container.cpp
  src/Formats/Optical/WiiU/Reader.cpp
  src/Formats/Optical/WiiU/Writer.cpp
  src/Formats/Optical/Xdvdfs/Reader.cpp
  src/Formats/Optical/Xdvdfs/Writer.cpp
  src/Formats/Optical/Alcohol/Descriptor.cpp
  src/Formats/Optical/Alcohol/Reader.cpp
  src/Formats/Optical/CloneCd/Descriptor.cpp
  src/Formats/Optical/CloneCd/Reader.cpp
  src/Formats/Common/Audio/Vorbis/PcmSource.cpp
  src/Formats/Optical/BinCue/Reader.cpp
  src/Formats/Optical/BinCue/Writer.cpp
  src/Formats/Optical/BinCue/WaveSource.cpp
  src/Formats/Optical/Gdi/Descriptor.cpp
  src/Formats/Optical/Gdi/Reader.cpp
  src/Formats/Optical/Gdi/Writer.cpp
  src/Formats/Common/Chd/Sources.cpp
  src/Formats/Optical/Chd/Metadata.cpp
  src/Formats/Optical/Chd/Layout.cpp
  src/Formats/Optical/Chd/Reader.cpp
  src/Formats/HardDisk/Chd/Layout.cpp
  src/Formats/HardDisk/Chd/Reader.cpp
  src/Formats/HardDisk/Chd/Writer.cpp
  src/Formats/Common/Chd/Container.cpp
  src/Formats/Common/Chd/Map.cpp
  src/Formats/Common/Chd/ContainerCodecs.cpp
  src/Formats/Common/Chd/Huffman.cpp
  src/Formats/Common/Chd/Compression.cpp
  src/Formats/Common/Chd/Flac.cpp
  src/Primitives/Crc32C.cpp
  src/Formats/HardDisk/Vhdx/Structures.cpp
  src/Formats/HardDisk/Vhdx/Metadata.cpp
  src/Formats/HardDisk/Vhdx/Reader.cpp
  src/Formats/HardDisk/Vhdx/StructuresWriter.cpp
  src/Formats/HardDisk/Vhdx/Writer.cpp
  src/Formats/HardDisk/Vmdk/Descriptor.cpp
  src/Formats/HardDisk/Vmdk/Sparse.cpp
  src/Formats/HardDisk/Vmdk/Reader.cpp
  src/Formats/HardDisk/Vmdk/SparseWriter.cpp
  src/Formats/HardDisk/Vmdk/Writer.cpp
  src/Formats/HardDisk/Qcow2/Header.cpp
  src/Formats/HardDisk/Qcow2/Refcounts.cpp
  src/Formats/HardDisk/Qcow2/Reader.cpp
  src/Formats/HardDisk/Qcow2/Layout.cpp
  src/Formats/HardDisk/Qcow2/Writer.cpp
  src/Formats/HardDisk/MfmEmu/Header.cpp
  src/Formats/HardDisk/MfmEmu/Signal.cpp
  src/Formats/HardDisk/MfmEmu/Container.cpp
  src/Formats/HardDisk/MfmEmu/GeneratedData.cpp
  src/Formats/HardDisk/MfmEmu/Representation.cpp
  src/Formats/HardDisk/MfmEmu/Reader.cpp
  src/Formats/HardDisk/MfmEmu/Writer.cpp
  src/Formats/HardDisk/MfmEmu/Creation.cpp
  src/Formats/HardDisk/Adaptec/ErrorCheck.cpp
  src/Formats/HardDisk/Adaptec/Encoding.cpp
  src/Formats/HardDisk/Adaptec/Decoding.cpp
  src/Formats/HardDisk/Adaptec/WriteWindow.cpp
  src/Formats/HardDisk/Adaptec/Geometry.cpp
  src/Formats/HardDisk/Adaptec/SectorReader.cpp
  src/Formats/HardDisk/Adaptec/Parameters.cpp
  src/Formats/HardDisk/Acorn/Descriptor.cpp
  src/Formats/HardDisk/Acorn/Reader.cpp
  src/Formats/HardDisk/Acorn/Writer.cpp
  src/Formats/HardDisk/Variant.cpp
  src/Formats/HardDisk/Vhd/Header.cpp
  src/Formats/HardDisk/Vhd/Reader.cpp
  src/Formats/HardDisk/Vhd/Geometry.cpp
  src/Formats/HardDisk/Vhd/Writer.cpp
  src/Formats/HardDisk/Vdi/Header.cpp
  src/Formats/HardDisk/Vdi/Reader.cpp
  src/Formats/HardDisk/Vdi/Writer.cpp
  src/Formats/KryoFlux/Metadata.cpp
  src/Formats/KryoFlux/Decoding.cpp
  src/Formats/KryoFlux/Files.cpp
  src/Formats/KryoFlux/Adapter.cpp
  src/Formats/HxcStream/Reading.cpp
  src/Formats/HxcStream/Pulses.cpp
  src/Formats/HxcStream/Revolutions.cpp
  src/Formats/HxcStream/Metadata.cpp
  src/Formats/HxcStream/Files.cpp
  src/Formats/HxcStream/Adapter.cpp
  src/Formats/HxcStream/Writer.cpp

  src/Formats/HxcStream/Encoding.cpp
  src/Formats/HxcStream/Blocks.cpp
  src/Formats/HxcStream/Digest.cpp
  src/Formats/I86f/Reader.cpp
  src/Formats/I86f/ProtectedImage.cpp
  src/Formats/I86f/Adapter.cpp
  src/Formats/Cp2/Layout.cpp
  src/Formats/Cp2/Reader.cpp
  src/Formats/Cp2/Adapter.cpp
  src/Formats/Jfd/Create.cpp
  src/Formats/Jfd/Reader.cpp
  src/Formats/Jfd/Writer.cpp
  src/Formats/Jfd/Metadata.cpp
  src/Formats/Jfd/Decode.cpp
  src/Formats/Jfd/Rewrite.cpp
  src/Formats/Jfd/Compression.cpp
  src/Formats/Catalog/Catalogue.cpp
  src/Formats/Catalog/Runtime.cpp
  src/Formats/Catalog/Greaseweazle.cpp
  src/Formats/Definitions/Layout.cpp
  src/Formats/Definitions/Reader.cpp
  src/Formats/Definitions/RawImages.cpp
  src/Sources/DriveRandomAccessData.cpp
  src/Sources/UnavailableRandomAccessData.cpp
  src/Models/Optical/Track.cpp
  src/Models/Optical/Image.cpp
  src/Models/Optical/DriveBinding.cpp
  src/Models/Sectors/Representation.cpp
  src/Models/Blocks/Ranges.cpp
  src/Models/Blocks/Reader.cpp
  src/Models/Blocks/DriveBinding.cpp
  src/Models/Blocks/Copy.cpp
  src/Images/MediaDocument.cpp
  src/Images/RecognitionContext.cpp
  src/Images/Writers.cpp
  src/Images/WriterRegistry.cpp
  src/Formats/HardDisk/Raw/Reader.cpp
  src/Formats/HardDisk/Raw/Writer.cpp
  src/Images/ReaderRegistry.cpp
  src/Images/RecognitionResult.cpp
  src/Images/ExplorationMapper.cpp
  src/Images/MediaExplorer.cpp
  src/Images/ReadRejection.cpp
  src/Images/DocumentFactory.cpp
  src/Images/WriterAdapters.cpp
  src/Models/Flux/Representation.cpp
  src/Formats/CommodoreDisks/Adapters.cpp
  src/Formats/Atari/Adapters.cpp
  src/Formats/ImageDisk/Adapter.cpp
  src/Formats/CpcDsk/Adapter.cpp
  src/Formats/Hfe/Adapter.cpp
  src/Formats/St/Transform.cpp
  src/Formats/St/Writer.cpp
  src/Formats/St/Detection.cpp
  src/Formats/Optical/Iso/Reader.cpp
  src/Formats/Optical/Iso/Writer.cpp
  src/Formats/Cartridge/Cpr.cpp
  src/Formats/Cartridge/Rom.cpp
  src/Formats/Cartridge/Fds.cpp
  src/Formats/Cartridge/Writers.cpp
  src/Formats/Raw/WriterAdapters.cpp
  src/Formats/Commodore900/WriterAdapter.cpp
  src/Formats/AcornAtom/WriterAdapter.cpp
  src/Formats/Raw/UcsdWriter.cpp
  src/Formats/TeleDisk/Writer.cpp
  src/Formats/TeleDisk/HeaderFactory.cpp
  src/Formats/TeleDisk/Reader.cpp
  src/Formats/TeleDisk/SectorImage.cpp
  src/Formats/TeleDisk/Adapter.cpp
  src/Formats/TeleDisk/SectorCodec.cpp
  src/Formats/TeleDisk/Advanced.cpp
  # Codecs
  src/Codecs/Crc16.cpp
  src/Codecs/FmAddressMarks.cpp
  src/Codecs/RotatingChecksum.cpp
  # Codecs/Aed6200p
  src/Codecs/Aed6200p/Decoder.cpp
  src/Codecs/Aed6200p/Encoder.cpp
  src/Codecs/Aed6200p/Fields.cpp
  # Codecs/Amiga
  src/Codecs/Amiga/Decoder.cpp
  src/Codecs/Amiga/Encoder.cpp
  src/Codecs/Amiga/OddEven.cpp
  src/Codecs/Amiga/Parity.cpp
  # Codecs/Apple
  src/Codecs/Apple/BitLatch.cpp
  # Codecs/AppleII
  src/Codecs/AppleII/Address.cpp
  src/Codecs/AppleII/Decoder.cpp
  src/Codecs/AppleII/Encoder.cpp
  src/Codecs/AppleII/FiveAndThree.cpp
  src/Codecs/AppleII/SixAndTwo.cpp
  # Codecs/AppleIwm
  src/Codecs/AppleIwm/Decode.cpp
  src/Codecs/AppleIwm/Decoder.cpp
  src/Codecs/AppleIwm/Encode.cpp
  src/Codecs/AppleIwm/Encoder.cpp
  # Codecs/AppleRwts18
  src/Codecs/AppleRwts18/Decoder.cpp
  src/Codecs/AppleRwts18/Encoder.cpp
  src/Codecs/AppleRwts18/Payload.cpp
  # Codecs/Arburg
  src/Codecs/Arburg/Checksum.cpp
  src/Codecs/Arburg/Decoder.cpp
  src/Codecs/Arburg/Encoder.cpp
  src/Codecs/Arburg/System.cpp
  # Codecs/Centurion
  src/Codecs/Centurion/Decoder.cpp
  src/Codecs/Centurion/Encoder.cpp
  src/Codecs/Centurion/Fields.cpp
  # Codecs/Commodore
  src/Codecs/Commodore/Commodore1541Decoder.cpp
  src/Codecs/Commodore/Commodore1541Encoder.cpp
  src/Codecs/Commodore/Commodore900Decoder.cpp
  src/Codecs/Commodore/Commodore900Encoder.cpp
  src/Codecs/Commodore/Gcr.cpp
  # Codecs/DataGeneral
  src/Codecs/DataGeneral/Checksum.cpp
  src/Codecs/DataGeneral/Decoder.cpp
  src/Codecs/DataGeneral/Encoder.cpp
  # Codecs/DecRx02
  src/Codecs/DecRx02/Decoder.cpp
  src/Codecs/DecRx02/Descriptions.cpp
  src/Codecs/DecRx02/Encoder.cpp
  src/Codecs/DecRx02/Format.cpp
  src/Codecs/DecRx02/M2Fm.cpp
  # Codecs/Emu
  src/Codecs/Emu/Decoder.cpp
  src/Codecs/Emu/Encoder.cpp
  # Codecs/Heathkit
  src/Codecs/Heathkit/Decoder.cpp
  src/Codecs/Heathkit/Encoder.cpp
  src/Codecs/Heathkit/Record.cpp
  # Codecs/HpMmfm
  src/Codecs/HpMmfm/Decoder.cpp
  src/Codecs/HpMmfm/Encoder.cpp
  src/Codecs/HpMmfm/Payload.cpp
  # Codecs/Iso
  src/Codecs/Iso/FmDecoder.cpp
  src/Codecs/Iso/FmEncoder.cpp
  src/Codecs/Iso/FmFields.cpp
  src/Codecs/Iso/FmRead.cpp
  src/Codecs/Iso/FmTiming.cpp
  src/Codecs/Iso/MfmDecoder.cpp
  src/Codecs/Iso/MfmEncoder.cpp
  src/Codecs/Iso/MfmFields.cpp
  src/Codecs/Iso/MfmPll.cpp
  src/Codecs/Iso/MfmRead.cpp
  # Codecs/Membrain
  src/Codecs/Membrain/Address.cpp
  src/Codecs/Membrain/Decoder.cpp
  src/Codecs/Membrain/Encoder.cpp
  src/Codecs/Membrain/Fields.cpp
  # Codecs/MicralN
  src/Codecs/MicralN/Checksum.cpp
  src/Codecs/MicralN/Decoder.cpp
  src/Codecs/MicralN/Encoder.cpp
  # Codecs/Micropolis
  src/Codecs/Micropolis/Decoder.cpp
  src/Codecs/Micropolis/Encoder.cpp
  src/Codecs/Micropolis/Record.cpp
  # Codecs/Northstar
  src/Codecs/Northstar/Address.cpp
  src/Codecs/Northstar/Decoder.cpp
  src/Codecs/Northstar/Encoder.cpp
  # Codecs/Raw
  src/Codecs/Raw/Raw.cpp
  # Codecs/Tycom
  src/Codecs/Tycom/Decoder.cpp
  src/Codecs/Tycom/Encoder.cpp
  src/Codecs/Tycom/Fields.cpp
  # Codecs/Victor9000
  src/Codecs/Victor9000/Decoder.cpp
  src/Codecs/Victor9000/Encoder.cpp
  src/Codecs/Victor9000/Fields.cpp
  # Codecs/QdMo5
  src/Codecs/QdMo5/Decoder.cpp
  src/Codecs/QdMo5/Encoder.cpp
  src/Codecs/QdMo5/Fields.cpp
  # Common
  src/Formats/Tape/AtomTap/Reader.cpp
  src/Formats/Tape/AtomTap/Reconstruction.cpp
  src/Formats/Tape/AtomTap/Decode.cpp
  src/Formats/Tape/AtomTap/Serial.cpp
  src/Formats/Tape/AtomTap/Encode.cpp
  src/Formats/Tape/AtomTap/Writer.cpp
  src/Formats/Tape/Uef/Reader.cpp
  src/Formats/Tape/Uef/Writer.cpp
  src/Formats/Tape/Uef/Compression.cpp
  src/Formats/Tape/Csw/Reader.cpp
  src/Formats/Tape/Csw/Writer.cpp
  src/Formats/Tape/Csw/Compression.cpp
  src/Formats/Tape/AtariCas/Reader.cpp
  src/Formats/Tape/AtariCas/Writer.cpp
  src/Decoding/Sequential/Compatibility.cpp
  src/Decoding/Sequential/Fsk/Result.cpp
  src/Decoding/Sequential/Fsk/Decode.cpp
  src/Decoding/Sequential/Fsk/Encode.cpp
  src/Decoding/Sequential/Atari/Records.cpp
  src/Decoding/Sequential/Atari/Decoder.cpp
  src/Encoding/Sequential/Request.cpp
  src/Encoding/Sequential/Registry.cpp
  src/Encoding/Sequential/Compatibility.cpp
  src/Encoding/Sequential/Atari/Records.cpp
  src/Formats/Tape/Amstrad/Records.cpp
  src/Formats/Tape/Common/PulseIntervals.cpp
  src/Decoding/Sequential/Amstrad/Decoder.cpp
  src/Decoding/Sequential/Amstrad/Pulses.cpp
  src/Encoding/Sequential/Amstrad/Encoder.cpp
  src/Formats/Tape/CommodoreTap/Profile.cpp
  src/Formats/Tape/CommodoreTap/Reader.cpp
  src/Formats/Tape/CommodoreTap/Writer.cpp
  src/Formats/Tape/CommodoreTap/Protocol.cpp
  src/Decoding/Sequential/Commodore/Decoder.cpp
  src/Encoding/Sequential/Commodore/Encoder.cpp
  src/Decoding/Sequential/Spectrum/Decoder.cpp
  src/Decoding/Sequential/Spectrum/Pulses.cpp
  src/Decoding/Sequential/Msx/Decoder.cpp
  src/Encoding/Sequential/Spectrum/Encoder.cpp
  src/Formats/Tape/Tzx/Metadata.cpp
  src/Decoding/Sequential/Tzx/SignalDecoder.cpp
  src/Decoding/Sequential/Tzx/Expansion.cpp
  src/Encoding/Sequential/Tzx/SignalEncoder.cpp
  src/Formats/Tape/SpectrumTap/Reader.cpp
  src/Formats/Tape/SpectrumTap/Writer.cpp
  src/Formats/Tape/Tzx/Reader.cpp
  src/Formats/Tape/Tzx/Blocks.cpp
  src/Formats/Tape/Tzx/Writer.cpp
  src/Formats/Tape/MsxCas/Reader.cpp
  src/Formats/Tape/MsxCas/Writer.cpp
  src/Encoding/Sequential/Msx/Encoder.cpp
  src/Formats/Tape/Acorn/Blocks.cpp
  src/Encoding/Sequential/Acorn/AcornEncoder.cpp
  src/Encoding/Sequential/Acorn/AcornOutput.cpp
  src/Decoding/Sequential/Acorn/AcornDecoder.cpp
  src/Encoding/Sequential/Acorn/AtomEncoder.cpp
  src/Decoding/Sequential/Acorn/AtomDecoder.cpp
  src/Encoding/Sequential/Atari/Encoder.cpp
  src/Decoding/Sequential/Models.cpp
  src/Decoding/Sequential/Registry.cpp
  src/Images/SequentialContentAdapter.cpp
  src/Diagnostics.cpp
  # Composition
  src/Composition/FluxCodecs.cpp
  src/Composition/SectorPolicies.cpp
  # Decoding
  src/Decoding/Confidence.cpp
  src/Decoding/LocalizedText.cpp
  src/Decoding/Registry.cpp
  src/Decoding/Scoring.cpp
  # Encoding
  src/Encoding/Bits.cpp
  src/Encoding/ProtectedImage.cpp
  src/Encoding/RareProfiles.cpp
  src/Encoding/Registry.cpp
  src/Encoding/SectorImageEncoder.cpp
  src/Encoding/Timings.cpp
  src/Encoding/Track.cpp
  # Exploration
  src/Exploration/SectorExploration.cpp
  src/Formats/TwoImg/Reader.cpp
  src/Formats/TwoImg/Writer.cpp
  src/Formats/Nib/Reader.cpp
  src/Formats/DiskCopy/Reader.cpp
  src/Formats/DiskCopy/Writer.cpp
  src/Formats/DiskCopy/Metadata.cpp
  src/Formats/Woz/Reader.cpp
  src/Formats/Woz/Writer.cpp
  src/Formats/Apple/DecodedTracks.cpp
  src/Formats/Msa/Rle.cpp
  src/Formats/Msa/Reader.cpp
  src/Formats/Msa/Writer.cpp
  src/Formats/St/Geometry.cpp
  src/Formats/Atari8Bit/Geometry.cpp
  src/Formats/Atr/Layout.cpp
  src/Formats/Atr/Reader.cpp
  src/Formats/Atr/Writer.cpp
  src/Formats/Atx/Reader.cpp
  src/Formats/Epson/Geometry.cpp
  src/Formats/ImageDisk/Models.cpp
  src/Formats/ImageDisk/Reader.cpp
  src/Formats/ImageDisk/Writer.cpp
  src/Formats/ImageDisk/Metadata.cpp
  src/Formats/ImageDisk/Builder.cpp
  src/Formats/CpcDsk/Reader.cpp
  src/Formats/CpcDsk/Builder.cpp
  src/Formats/CpcDsk/Writer.cpp
  src/Models/Sectors/UniformLayout.cpp
  src/Models/Flux/Conversion.cpp
  src/Formats/Hfe/Models.cpp
  src/Formats/Hfe/Reader.cpp
  src/Formats/Hfe/Writer.cpp
  src/Formats/Hfe/ProtectedImage.cpp
  src/Formats/Hfe/V3Writer.cpp
  src/Encoding/BitPacker.cpp
  src/Formats/Apple/TrackEncoding.cpp
  src/Formats/Nib/Writer.cpp
  # Formats/Apple
  src/Formats/Apple/Geometry.cpp
  src/Formats/Apple/Probe.cpp
  src/Formats/Apple/RawReader.cpp
  src/Formats/Adf/Adapter.cpp
  src/Formats/BbcDfs/Adapter.cpp
  src/Formats/Raw/Adapters.cpp
  src/Formats/Raw/ImgReader.cpp
  src/Formats/Xfd/Reader.cpp
  src/Formats/Apridisk/Reader.cpp
  src/Formats/Apridisk/Writer.cpp
  src/Formats/Apridisk/Adapter.cpp
  src/Formats/Rx02/Reader.cpp
  src/Formats/Rx02/Writer.cpp
  src/Formats/Rx02/Adapter.cpp
  src/Formats/AcornAtom/Reader.cpp
  src/Formats/Raw/SpecializedReaders.cpp
  src/Formats/Epson/RawReader.cpp
  src/Formats/Epson/RawWriter.cpp
  src/Formats/Epson/WriterAdapter.cpp
  src/Formats/ImageDisk/WriterAdapter.cpp
  src/Formats/Adf/WriterAdapters.cpp
  src/Formats/BbcDfs/WriterAdapter.cpp
  src/Formats/CpcDsk/WriterAdapter.cpp
  src/Formats/Apple/Writing.cpp
  src/Formats/Apple/WriterAdapters.cpp
  src/Formats/Apple/Adapter.cpp
  src/Formats/Apple/ContainerRouter.cpp
  src/Formats/Apple/RawWriter.cpp
  src/Formats/Apple/SectorOrder.cpp
  # Formats/Adf
  src/Formats/Adf/Geometry.cpp
  src/Formats/Adf/Partial.cpp
  src/Formats/Adf/Reader.cpp
  src/Formats/Adf/Writer.cpp
  # Formats/BbcDfs
  src/Formats/BbcDfs/Reader.cpp
  src/Formats/BbcDfs/Writer.cpp
  # Formats/Commodore900
  src/Formats/Commodore900/FluxReader.cpp
  src/Formats/Commodore900/Geometry.cpp
  src/Formats/Commodore900/Reader.cpp
  src/Formats/Commodore900/Writer.cpp
  # Formats/CommodoreDisks
  src/Formats/CommodoreDisks/FluxReader.cpp
  src/Formats/CommodoreDisks/GcrReconstruction.cpp
  src/Formats/CommodoreDisks/Layouts.cpp
  src/Formats/CommodoreDisks/MfmReconstruction.cpp
  src/Formats/CommodoreDisks/Reader.cpp
  src/Formats/CommodoreDisks/Router.cpp
  src/Formats/CommodoreDisks/Writer.cpp
  # Formats/Raw
  src/Formats/Raw/Geometry.cpp
  src/Formats/Raw/Reader.cpp
  src/Formats/Raw/Writer.cpp
  # Formats/Scp
  src/Formats/Scp/DiskTypes.cpp
  src/Formats/Scp/SectorConverter.cpp
  src/Formats/Scp/Models.cpp
  src/Formats/Scp/ProtectedImage.cpp
  src/Formats/Scp/Adapter.cpp
  src/Formats/Scp/FileCache.cpp
  src/Formats/Scp/Metadata.cpp
  src/Formats/Scp/OutputImage.cpp
  src/Formats/Scp/Reader.cpp
  src/Formats/Scp/Synthetic.cpp
  src/Formats/Scp/Windows.cpp
  src/Formats/Scp/Writer.cpp
  # Models/Flux
  src/Models/Flux/BitReader.cpp
  src/Models/Flux/Bitstream.cpp
  src/Models/Flux/ProtectedTrack.cpp
  src/Models/Flux/ProtectedTrackImage.cpp
  src/Models/Flux/Revolution.cpp
  src/Models/Flux/Timing.cpp
  src/Models/Flux/Transitions.cpp
  # Models/Sectors
  src/Models/Sectors/LinearSectorWriter.cpp
  src/Models/Sectors/SectorImages.cpp
  src/Models/Sectors/SizeCodes.cpp
  # Reconstruction/Iso
  src/Reconstruction/Iso/Builder.cpp
  src/Reconstruction/Iso/Candidates.cpp
  src/Reconstruction/Iso/CaptureCache.cpp
  src/Reconstruction/Iso/Collection.cpp
  src/Reconstruction/Iso/Collector.cpp
  src/Reconstruction/Iso/Measurement.cpp
  src/Reconstruction/Iso/Normalizer.cpp
  src/Reconstruction/Iso/Score.cpp
  # Reconstruction/Sector
  src/Reconstruction/Sector/Policy.cpp
  src/Reconstruction/Sector/Matching.cpp
  src/Reconstruction/Sector/Apple.cpp
  src/Reconstruction/Sector/Commodore.cpp
  src/Reconstruction/Sector/OtherFamilies.cpp
  src/Reconstruction/Sector/Registry.cpp
  # Visualization
  src/Visualization/SectorFluxVisualizer.cpp
  # Sources
  src/Sources/AtomicImageSetWriter.cpp
  src/Sources/AtomicImageWriter.cpp
  src/Sources/RandomAccessSources.cpp
  src/Sources/FileIdentity.cpp)
target_compile_definitions(RetroMediaLab.MediaEngine PRIVATE RETROMEDIALAB_MEDIAENGINE_BUILD)
target_include_directories(RetroMediaLab.MediaEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(RetroMediaLab.MediaEngine PRIVATE Qt6::Core Qt6::Multimedia RetroMediaLab.Tremor RetroMediaLab.MediaFileSystems zlibstatic lz4_static liblzma libzstd_static)
target_include_directories(RetroMediaLab.MediaEngine PRIVATE ${PROJECT_SOURCE_DIR}/third_party/xz/src/liblzma/api)
set_target_properties(RetroMediaLab.MediaEngine PROPERTIES OUTPUT_NAME MediaEngine PREFIX "")
install(TARGETS RetroMediaLab.MediaEngine RUNTIME DESTINATION lib LIBRARY DESTINATION lib)


qt_add_resources(RetroMediaLab.MediaEngine DiskDefinitionsData
  PREFIX "/MediaEngine/DiskDefinitions" BASE src/Formats/Definitions/Data
  FILES
    src/Formats/Definitions/Data/diskdefs.cfg
    src/Formats/Definitions/Data/diskdefs_acorn.cfg
    src/Formats/Definitions/Data/diskdefs_akai.cfg
    src/Formats/Definitions/Data/diskdefs_amiga.cfg
    src/Formats/Definitions/Data/diskdefs_apple2.cfg
    src/Formats/Definitions/Data/diskdefs_atari.cfg
    src/Formats/Definitions/Data/diskdefs_atarist.cfg
    src/Formats/Definitions/Data/diskdefs_coco.cfg
    src/Formats/Definitions/Data/diskdefs_commodore.cfg
    src/Formats/Definitions/Data/diskdefs_datageneral.cfg
    src/Formats/Definitions/Data/diskdefs_dec.cfg
    src/Formats/Definitions/Data/diskdefs_dragon.cfg
    src/Formats/Definitions/Data/diskdefs_eagle.cfg
    src/Formats/Definitions/Data/diskdefs_ensoniq.cfg
    src/Formats/Definitions/Data/diskdefs_epson.cfg
    src/Formats/Definitions/Data/diskdefs_gem.cfg
    src/Formats/Definitions/Data/diskdefs_hp.cfg
    src/Formats/Definitions/Data/diskdefs_ibm.cfg
    src/Formats/Definitions/Data/diskdefs_kaypro.cfg
    src/Formats/Definitions/Data/diskdefs_luxor.cfg
    src/Formats/Definitions/Data/diskdefs_mac.cfg
    src/Formats/Definitions/Data/diskdefs_micropolis.cfg
    src/Formats/Definitions/Data/diskdefs_mm1.cfg
    src/Formats/Definitions/Data/diskdefs_msx.cfg
    src/Formats/Definitions/Data/diskdefs_northstar.cfg
    src/Formats/Definitions/Data/diskdefs_occ1.cfg
    src/Formats/Definitions/Data/diskdefs_olivetti.cfg
    src/Formats/Definitions/Data/diskdefs_pc98.cfg
    src/Formats/Definitions/Data/diskdefs_raw.cfg
    src/Formats/Definitions/Data/diskdefs_sci.cfg
    src/Formats/Definitions/Data/diskdefs_sega.cfg
    src/Formats/Definitions/Data/diskdefs_thomson.cfg
    src/Formats/Definitions/Data/diskdefs_tsc.cfg
    src/Formats/Definitions/Data/diskdefs_xerox.cfg
    src/Formats/Definitions/Data/diskdefs_zx.cfg
)

qt_add_resources(RetroMediaLab.MediaEngine ImageFormatCatalogueData
  PREFIX "/MediaEngine/FormatCatalog" BASE src/Formats/Catalog/Data
  FILES src/Formats/Catalog/Data/formats.json)

if(WIN32)
  target_sources(RetroMediaLab.MediaEngine PRIVATE src/Sources/Platform/Windows/Preallocation.cpp)
else()
  target_sources(RetroMediaLab.MediaEngine PRIVATE src/Sources/Platform/PreallocationUnavailable.cpp)
endif()

install(DIRECTORY Assets/MechanicalAudio DESTINATION Assets)
