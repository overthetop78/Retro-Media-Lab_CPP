#pragma once
#include "Track.h"
#include <RetroMediaLab/MediaEngine/RepresentationInfo.h>
#include <RetroMediaLab/MediaEngine/DriveMechanics.h>
namespace RetroMediaLab::MediaEngine {
struct OpticalImageOptions {
    std::optional<std::int64_t> logicalLength;
    std::optional<std::vector<std::shared_ptr<const OpticalTrack>>> tracks;
    std::optional<std::int32_t> layerCount,faceCount;
    std::optional<std::vector<std::string>> associatedFiles;
};
struct OpticalTrackCoordinates {std::int32_t sessionNumber,trackNumber;std::int64_t firstSector,sectorCount;};
class OpticalImage final:public MediaFileSystems::OpticalRepresentation,public RepresentationInfo {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit OpticalImage(OpticalImageOptions);
    RETROMEDIALAB_MEDIAENGINE_API static std::shared_ptr<const OpticalImage> fromCoordinates(std::optional<std::int64_t>,std::vector<OpticalTrackCoordinates>,std::optional<std::int32_t> layers={},std::optional<std::int32_t> faces={},std::optional<std::vector<std::string>> associatedFiles={});
    const OpticalImageOptions& options()const noexcept{return options_;}
    const std::optional<std::vector<std::int32_t>>& sessions()const noexcept{return sessions_;}
    RepresentationKind representationKind()const noexcept override{return RepresentationKind::OpticalTracks;}
    std::optional<std::int64_t> logicalLength()const override{return options_.logicalLength;}
    bool supportsRandomAccess()const override{return true;}
    bool supportsSequentialAccess()const override{return true;}
    std::optional<std::span<const std::shared_ptr<const MediaFileSystems::OpticalTrack>>> tracks()const override{if(!tracks_)return {};return std::span<const std::shared_ptr<const MediaFileSystems::OpticalTrack>>(*tracks_);}
private:
    OpticalImageOptions options_;
    std::optional<std::vector<std::shared_ptr<const MediaFileSystems::OpticalTrack>>> tracks_;
    std::optional<std::vector<std::int32_t>> sessions_;
};
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const OpticalImage> bindOptical(std::shared_ptr<const OpticalImage>,std::shared_ptr<DriveMechanics>,bool writing);
}
