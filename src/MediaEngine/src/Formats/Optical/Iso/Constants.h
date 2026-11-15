#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::OpticalIsoConstants {
inline constexpr std::int32_t SectorSize=2048,FirstVolumeDescriptorSector=16;
inline constexpr std::string_view IsoExtension=".iso",GcmExtension=".gcm",WriterId="optical-iso",SectorSizeKey="sectorSize",TrackCountKey="trackCount",SourceExtensionKey="sourceExtension";
}
