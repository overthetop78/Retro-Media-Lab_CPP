#pragma once
#include "DiskExplorationTypes.h"
namespace RetroMediaLab::MediaEngine {
class MediaDocument;
enum class SectorVisualizationState {WithData,WithoutData,Degraded,Dead};
struct SectorVisualizationItem {
    std::size_t sourceIndex=0;
    std::uint64_t logicalBlock=0;
    std::int64_t cylinder=0,head=0,number=0;
    SectorVisualizationState state=SectorVisualizationState::WithoutData;
};
struct SectorVisualizationModel {
    bool logical=false;
    std::uint32_t cylinders=0,heads=0;
    std::vector<SectorVisualizationItem> sectors;
};
RETROMEDIALAB_MEDIAENGINE_API SectorVisualizationModel describeSectorVisualization(const DiskFormatInfo&,bool allowsVariableBlockSize,bool logicalAddressing);
RETROMEDIALAB_MEDIAENGINE_API SectorVisualizationModel describeSectorVisualization(std::shared_ptr<const MediaDocument>,const DiskFormatInfo&);
}
