#pragma once
#include "MediaVisualization.h"
#include "SequentialTypes.h"
#include <map>
namespace RetroMediaLab::MediaEngine {
enum class SequentialVisualizationKind {Unknown,Signal,Silence,DecodedBlock};
struct SequentialVisualizationSegment {
    SequentialVisualizationKind kind=SequentialVisualizationKind::Unknown;
    std::int32_t lane=0;
    double start=0,length=1;
    std::int64_t position=0;
    std::optional<std::int64_t> storedLength,startTicks,durationTicks;
    std::optional<std::int32_t> face,track,channel;
    std::map<std::string,std::string> metadata;
};
struct SequentialVisualizationModel {
    bool timed=false;
    double totalLength=0;
    std::optional<std::int64_t> logicalLength,durationTicks;
    std::vector<std::int32_t> lanes;
    std::vector<SequentialVisualizationSegment> segments;
    std::map<std::string,std::string> metadata;
};
RETROMEDIALAB_MEDIAENGINE_API SequentialVisualizationModel describeSequentialVisualization(std::shared_ptr<const MediaDocument>,const VisualizationDescriptor&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::uint8_t>> readSequentialVisualizationPreviewAsync(std::shared_ptr<const MediaDocument>,std::size_t,std::stop_token={});
}
