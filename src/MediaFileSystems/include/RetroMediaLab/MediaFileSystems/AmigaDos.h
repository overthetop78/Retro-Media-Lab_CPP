#pragma once
#include "Migration.h"
#include "SectorImage.h"
namespace RetroMediaLab::MediaFileSystems::AmigaDos {
enum class Variant:std::uint8_t {Ofs=0,Ffs,OfsInternational,FfsInternational,OfsDirectoryCache,FfsDirectoryCache,OfsLongNames,FfsLongNames};
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string_view fileSystemId(Variant) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool isFast(Variant) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool supportsLongNames(Variant) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API EntryKind entryKind(std::int32_t) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool validChecksum(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool unavailableBlock(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string readName(Bytes,std::int64_t offset,std::int64_t maximum);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string readEntryName(Bytes,Variant);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool validName(std::string_view);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<Timestamp> readTime(Bytes,std::int64_t offset);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token = {});
struct HardDiskVolume {
    std::vector<std::uint8_t> boot;
    std::uint64_t metadataOffset;
    std::vector<std::uint8_t> metadata;
};
RETROMEDIALAB_MEDIAFILESYSTEMS_API HardDiskVolume prepareHardDiskVolume(
    std::uint64_t capacity, Variant, std::string_view name, std::stop_token = {});
// defaultTime freezes the default metadata timestamp for a build; absent uses the current UTC clock.
RETROMEDIALAB_MEDIAFILESYSTEMS_API SectorWritePlan write(const MigrationPlan&,Variant,const SectorGeometry&,std::optional<Timestamp> defaultTime = {},std::stop_token = {});
}
