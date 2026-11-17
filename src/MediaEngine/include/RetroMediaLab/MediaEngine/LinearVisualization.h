#pragma once
#include "MediaVisualization.h"
#include "HardDiskGeometry.h"
#include "VolumeInfo.h"
namespace RetroMediaLab::MediaEngine {
enum class BlockVisualizationState {Unknown,Available,Allocated};
enum class OpticalVisualizationKind {Unknown,Data,Audio};
struct LinearVisualizationItem {
    BlockVisualizationState blockState=BlockVisualizationState::Unknown;
    OpticalVisualizationKind opticalKind=OpticalVisualizationKind::Unknown;
    std::optional<VolumeInfo> volume;
    std::optional<std::int64_t> pregap,postgap;
    std::optional<std::int32_t> storedSectorSize;
};
struct LinearVisualizationModel {
    std::int64_t logicalLength=0;
    std::int32_t blockSize=1;
    bool cartridge=false;
    std::optional<HardDiskGeometry> geometry;
    std::optional<std::int32_t> faces,layers;
    std::vector<std::string> associatedFiles;
    std::vector<LinearVisualizationItem> items;
};
RETROMEDIALAB_MEDIAENGINE_API LinearVisualizationModel describeLinearVisualization(std::shared_ptr<const MediaDocument>,const VisualizationDescriptor&,std::stop_token={});
// Reads only the beginning of the selected range or one stored optical sector.
RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::uint8_t>> readVisualizationPreviewAsync(std::shared_ptr<const MediaDocument>,VisualizationElement,std::stop_token={});
}
