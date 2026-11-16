add_library(RetroMediaLab.MediaFileSystems SHARED
  src/Exploration/Composition/Interpretation.cpp
  src/Migration/Planning.cpp
  src/Migration/Capabilities.cpp
  src/Migration/Service.cpp
  src/FileSystems/Apple/Sos/Writer.cpp
  # Common
  src/Diagnostics.cpp
  src/Sources.cpp
  src/FileSystems/Nintendo/GameCube/Reader.cpp
  src/FileSystems/Nintendo/FamicomDisk/Reader.cpp
  src/FileSystems/Cartridge/Readers.cpp
  src/FileSystems/Optical/Common.cpp
  src/FileSystems/Iso9660/Reader.cpp
  src/FileSystems/Iso9660/Names.cpp
  src/FileSystems/Xbox/Xdvdfs/Reader.cpp
  src/FileSystems/Udf/Descriptors.cpp
  src/FileSystems/Udf/Reader.cpp
  src/FileSystems/Udf/Entries.cpp
  src/Exploration/Sequential/Block.cpp
  src/Exploration/Sequential/Reader.cpp
  # Exploration
  src/Exploration/EntryAnalysis.cpp
  # FileSystems/Acorn
  src/FileSystems/Acorn/Adfs/BlockView.cpp
  src/FileSystems/Acorn/Adfs/Directory.cpp
  src/FileSystems/Acorn/Adfs/File.cpp
  src/FileSystems/Acorn/Adfs/Formatter.cpp
  src/FileSystems/Acorn/Adfs/Reader.cpp
  src/FileSystems/Acorn/Adfs/Time.cpp
  src/FileSystems/Acorn/BbcDfs/Codec.cpp
  src/FileSystems/Acorn/BbcDfs/Reader.cpp
  src/FileSystems/Acorn/BbcDfs/Sides.cpp
  src/FileSystems/Acorn/FileCore/Codec.cpp
  src/FileSystems/Acorn/FileCore/NewMap.cpp
  src/FileSystems/Acorn/FileCore/OldMap.cpp
  # FileSystems/Amiga
  src/FileSystems/Amiga/Dos/Codec.cpp
  src/FileSystems/Amiga/Dos/Directory.cpp
  src/FileSystems/Amiga/Dos/File.cpp
  src/FileSystems/Amiga/Dos/Reader.cpp
  src/FileSystems/Amiga/Dos/Recovery.cpp
  src/FileSystems/Amiga/Dos/Writer.cpp
  src/FileSystems/Amiga/FlatArchive/Data.cpp
  src/FileSystems/Amiga/FlatArchive/Directory.cpp
  src/FileSystems/Amiga/FlatArchive/Reader.cpp
  src/FileSystems/Apple/Dos/Codec.cpp
  src/FileSystems/Apple/Dos/Reader.cpp
  src/FileSystems/Apple/Dos/Writer.cpp
  src/FileSystems/Apple/ProDos/Codec.cpp
  src/FileSystems/Apple/ProDos/Reader.cpp
  src/FileSystems/Apple/ProDos/Directory.cpp
  src/FileSystems/Apple/ProDos/File.cpp
  src/FileSystems/Apple/ProDos/Bitmap.cpp
  src/FileSystems/Apple/ProDos/Writer.cpp
  src/FileSystems/Apple/Macintosh/Primitives.cpp
  src/FileSystems/Apple/Macintosh/Mfs/Allocation.cpp
  src/FileSystems/Apple/Macintosh/Mfs/Fork.cpp
  src/FileSystems/Apple/Macintosh/Mfs/Directory.cpp
  src/FileSystems/Apple/Macintosh/Mfs/Reader.cpp
  src/FileSystems/Apple/Macintosh/Hfs/Reader.cpp
  src/FileSystems/Apple/Macintosh/Hfs/Extent.cpp
  src/FileSystems/Apple/Macintosh/Hfs/Catalog.cpp
  src/FileSystems/Apple/Macintosh/Hfs/Directory.cpp
  src/FileSystems/Apple/Lisa/VolumeHeader.cpp
  src/FileSystems/Apple/Lisa/Tags.cpp
  src/FileSystems/Apple/Lisa/Catalog.cpp
  src/FileSystems/Apple/Lisa/Content.cpp
  src/FileSystems/Apple/Lisa/Reader.cpp
  # FileSystems/Apple
  src/FileSystems/Apple/Recognition.cpp
  # FileSystems/Commodore
  src/FileSystems/Commodore/Coherent/Codec.cpp
  src/FileSystems/Commodore/Coherent/Directory.cpp
  src/FileSystems/Commodore/Coherent/File.cpp
  src/FileSystems/Commodore/Coherent/Image.cpp
  src/FileSystems/Commodore/Coherent/Inode.cpp
  src/FileSystems/Commodore/Coherent/Reader.cpp
  src/FileSystems/Commodore/Dos/Allocation.cpp
  src/FileSystems/Commodore/Dos/Bam.cpp
  src/FileSystems/Commodore/Dos/Directory.cpp
  src/FileSystems/Commodore/Dos/File.cpp
  src/FileSystems/Commodore/Dos/Layout.cpp
  src/FileSystems/Commodore/Dos/Reader.cpp
  src/FileSystems/Commodore/Dos/Sector.cpp
  src/FileSystems/Commodore/Dos/WriteFiles.cpp
  src/FileSystems/Commodore/Dos/WriteGeometry.cpp
  src/FileSystems/Commodore/Dos/WritePolicy.cpp
  src/FileSystems/Commodore/Dos/WriteSystem.cpp
  src/FileSystems/Commodore/Dos/Writer.cpp
  src/FileSystems/Ucsd/Primitives.cpp
  src/FileSystems/Ucsd/Header.cpp
  src/FileSystems/Ucsd/Directory.cpp
  src/FileSystems/Ucsd/Content.cpp
  src/FileSystems/Ucsd/Reader.cpp
  src/FileSystems/Cpm/Layout.cpp
  src/FileSystems/Cpm/Directory.cpp
  src/FileSystems/Cpm/Content.cpp
  src/FileSystems/Cpm/Reader.cpp
  src/FileSystems/Dec/Rt11/Primitives.cpp
  src/FileSystems/Dec/Rt11/Content.cpp
  src/FileSystems/Dec/Rt11/Directory.cpp
  src/FileSystems/Dec/Rt11/Reader.cpp
  src/FileSystems/Atari/Dos/Primitives.cpp
  src/FileSystems/Atari/Dos/File.cpp
  src/FileSystems/Atari/Dos/Allocation.cpp
  src/FileSystems/Atari/Dos/Directory.cpp
  src/FileSystems/Atari/Dos/Reader.cpp
  src/FileSystems/Atari/SpartaDos/Primitives.cpp
  src/FileSystems/Atari/SpartaDos/Disk.cpp
  src/FileSystems/Atari/SpartaDos/Allocation.cpp
  src/FileSystems/Atari/SpartaDos/File.cpp
  src/FileSystems/Atari/SpartaDos/Directory.cpp
  src/FileSystems/Atari/SpartaDos/Reader.cpp
  src/Exploration/Composition/Catalog.cpp
  src/Exploration/Composition/Document.cpp
  src/Exploration/Composition/Identity.cpp
  src/Exploration/Composition/Registry.cpp
  src/FileSystems/Atari/FormatRecognition.cpp
  src/FileSystems/Atari/KFile/Reader.cpp
  src/FileSystems/Atari/BootDisk/Reader.cpp
  src/FileSystems/Atari/ClkGraphicsLibrary/Reader.cpp
  src/Exploration/Volumes/Detectors.cpp
  src/Exploration/Volumes/Registry.cpp
  src/Exploration/MediaExplorer.cpp
  # FileSystems/Fat
  src/FileSystems/Fat/BootAndNames.cpp
  src/FileSystems/Fat/Fat12/Directory.cpp
  src/FileSystems/Fat/Fat12/Layout.cpp
  src/FileSystems/Fat/Fat12/Reader.cpp
  src/FileSystems/Fat/Fat12/Tables.cpp
  src/FileSystems/Fat/Fat12/Writer.cpp
  # Formats/Commodore
  src/Formats/Commodore/Geometry.cpp
  # Partitioning
  src/Partitioning/Gpt.cpp
  src/Partitioning/Mbr.cpp
  src/Partitioning/Volumes.cpp
  # Primitives
  src/Primitives/Petscii.cpp
  src/Primitives/Timestamps.cpp)
target_compile_definitions(RetroMediaLab.MediaFileSystems PRIVATE RETROMEDIALAB_MEDIAFILESYSTEMS_BUILD)
target_include_directories(RetroMediaLab.MediaFileSystems PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(RetroMediaLab.MediaFileSystems PRIVATE Qt6::Core RetroMediaLab.MediaAnalysis)
set_target_properties(RetroMediaLab.MediaFileSystems PROPERTIES OUTPUT_NAME MediaFileSystems PREFIX "")
install(TARGETS RetroMediaLab.MediaFileSystems RUNTIME DESTINATION lib LIBRARY DESTINATION lib)
