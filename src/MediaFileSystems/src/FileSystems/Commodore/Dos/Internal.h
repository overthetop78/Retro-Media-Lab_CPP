#pragma once
#include <RetroMediaLab/MediaFileSystems/CommodoreDos.h>
#include <RetroMediaLab/MediaFileSystems/CommodoreGeometry.h>
#include "Constants.h"
#include "../../../Primitives/Petscii.h"
namespace RetroMediaLab::MediaFileSystems::CommodoreDos::Internal {
struct Layout {std::int32_t headerTrack,headerSector,directoryTrack,directorySector,nameOffset;std::uint8_t signature;bool d81=false;};
struct Image {const SectorImage& source;std::stop_token token;};
enum class SectorStatus {Success,InvalidCoordinate,Missing,Truncated};
struct SectorRead {SectorStatus status;std::shared_ptr<const SectorBlock> block;};
struct Recognition {Layout layout;std::vector<std::uint8_t> header;std::int32_t directoryTrack,directorySector;};
struct FileResult {std::vector<std::uint8_t> content;bool valid=true;std::vector<std::pair<std::int32_t,std::int32_t>> visited;std::vector<Diagnostic> warnings;std::optional<std::int32_t> firstBlock;};
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<Layout> layout(std::string_view);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<std::int32_t> logicalBlock(const SectorImage&,std::int32_t track,std::int32_t sector);
RETROMEDIALAB_MEDIAFILESYSTEMS_API SectorRead sector(const Image&,std::int32_t track,std::int32_t sector);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool plausible(const Image&,std::int32_t track,std::int32_t sector);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<Recognition> recognize(const Image&);
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileResult file(const Image&,std::int32_t track,std::int32_t sector,std::vector<Diagnostic>&,const std::string& name);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::vector<FileSystemEntry> directory(const Image&,std::int32_t track,std::int32_t sector,std::vector<Diagnostic>&);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<std::int32_t> freeBlocks(const Image&,bool d81,std::vector<Diagnostic>&);
}
