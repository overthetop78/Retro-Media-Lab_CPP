#pragma once
#include "Fat.h"
#include "Migration.h"
namespace RetroMediaLab::MediaFileSystems::Fat12 {
struct Layout {
    std::uint32_t reservedSectors, sectorsPerFat, rootStart, rootSectors, dataStart, sectorsPerCluster, clusterCount, fatCount = 1;
};
struct TargetGeometry { SectorGeometry geometry; bool isMsx = false; };
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<Layout> readLayout(Bytes boot, std::uint64_t availableSectors, std::string_view formatId);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<std::uint16_t> readTableEntry(Bytes, std::uint32_t cluster);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool plausibleTableHeader(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool validShortName(std::string_view);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool validVolumeName(std::string_view);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&, std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&, std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API SectorWritePlan write(const MigrationPlan&, const TargetGeometry&, std::stop_token = {});
}
