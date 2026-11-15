#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <functional>
#include <vector>
#include <string>
#include <optional>
#include <span>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Epson {
struct TrackGeometry {std::int32_t firstSector=0,count=0,sectorSize=0;bool operator==(const TrackGeometry&)const=default;};
struct Geometry {std::string formatId;std::int32_t cylinders=0,heads=0;std::function<TrackGeometry(std::int32_t,std::int32_t)> track;RETROMEDIALAB_MEDIAENGINE_API std::vector<TrackGeometry> allTracks()const;};
struct SectorDescriptor {std::int32_t cylinder=0,head=0,number=0,size=0;};
RETROMEDIALAB_MEDIAENGINE_API Geometry uniform(std::string,std::int32_t,std::int32_t,TrackGeometry);
RETROMEDIALAB_MEDIAENGINE_API const std::vector<Geometry>& geometries();
RETROMEDIALAB_MEDIAENGINE_API const Geometry& resolve(std::string_view);
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::string> detect(std::span<const SectorDescriptor>);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::string> scpCandidateFormatIds();
}
