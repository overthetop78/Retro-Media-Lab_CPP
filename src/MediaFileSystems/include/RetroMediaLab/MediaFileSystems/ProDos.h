#pragma once
#include "Migration.h"
#include "SectorImage.h"
#include <span>
namespace RetroMediaLab::MediaFileSystems::ProDos {
enum class Storage:std::uint8_t {Inactive=0,Seedling=1,Sapling=2,Tree=3,Subdirectory=0x0d,SubdirectoryHeader=0x0e,VolumeHeader=0x0f};
enum class FileType:std::uint8_t {Text=0x04,Binary=0x06,Directory=0x0f,Basic=0xfc,Variables=0xfd,System=0xff};
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool validName(std::string_view) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string readName(Bytes,std::int64_t offset);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<Timestamp> readTime(Bytes,std::int64_t offset);
RETROMEDIALAB_MEDIAFILESYSTEMS_API void writeTime(MutableBytes,std::int64_t offset,const std::optional<Timestamp>&);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&,std::stop_token={});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token={});
RETROMEDIALAB_MEDIAFILESYSTEMS_API SectorWritePlan write(const MigrationPlan&,const SectorGeometry&,std::uint32_t logicalBlockCount,std::span<const SectorAddress>,std::optional<Timestamp> defaultTime={},std::stop_token={});
}
