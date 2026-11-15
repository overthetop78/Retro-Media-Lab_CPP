#pragma once
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <RetroMediaLab/MediaEngine/Export.h>
namespace RetroMediaLab::MediaEngine::St {
enum class GeometryEvidence {Bpb,CapacityFallback};
struct GeometryDetection {MediaFileSystems::SectorGeometry geometry;GeometryEvidence evidence;};
RETROMEDIALAB_MEDIAENGINE_API std::string formatId(std::int64_t capacity);
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::SectorGeometry geometry(std::int32_t cylinders,std::int32_t heads,std::int32_t sectors);
RETROMEDIALAB_MEDIAENGINE_API std::optional<MediaFileSystems::SectorGeometry> geometry(std::string_view formatId);
RETROMEDIALAB_MEDIAENGINE_API GeometryDetection detect(MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes,std::stop_token={});
}
