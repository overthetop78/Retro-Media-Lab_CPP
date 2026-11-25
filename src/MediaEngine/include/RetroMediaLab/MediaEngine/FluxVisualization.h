#pragma once
#include "DiskExplorationTypes.h"
#include <stop_token>
#include <functional>
namespace RetroMediaLab::MediaEngine {
enum class FluxVisualMark {Short,Normal,Long,Header,Data,Deleted,Timing,Other};
struct FluxVisualArc {double start=0,sweep=0;FluxVisualMark mark=FluxVisualMark::Normal;};
struct FluxVisualRevolution {std::int32_t number=0;std::size_t transitions=0;double milliseconds=0,rpm=0,quality=0;std::vector<FluxVisualArc> arcs;};
struct FluxVisualTrack {std::int32_t head=0,cylinder=0;std::optional<std::int32_t> sourceNumber;std::vector<FluxVisualRevolution> revolutions;FluxVisualRevolution synthesis;};
struct FluxVisualizationModel {std::vector<FluxVisualTrack> tracks;std::vector<std::string> decoders;};
struct FluxVisualStructure {std::string kindKey;std::uint64_t bitOffset=0,bitLength=0;};
struct FluxTrackInspection {std::string decoderId;double confidence=0,cellTicks=0;std::size_t revolution=0;std::vector<FluxVisualArc> structures;std::vector<FluxVisualStructure> details;std::vector<DiskSectorInfo> sectors;};
RETROMEDIALAB_MEDIAENGINE_API FluxVisualizationModel describeFluxVisualization(const DiskImageInfo&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API FluxVisualizationModel describeFluxVisualization(const DiskImageInfo&,std::stop_token,const std::function<void(const FluxVisualTrack&)>&);
RETROMEDIALAB_MEDIAENGINE_API FluxTrackInspection inspectFluxVisualization(const DiskTrackInfo&,std::optional<std::string> decoder={},std::stop_token={},std::optional<std::size_t> revolution={});
}
