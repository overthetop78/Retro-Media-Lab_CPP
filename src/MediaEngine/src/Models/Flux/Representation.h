#pragma once
#include "ProtectedTrack.h"
#include <RetroMediaLab/MediaEngine/RepresentationInfo.h>
#include <RetroMediaLab/MediaFileSystems/Representations.h>
namespace RetroMediaLab::MediaEngine {
class FluxRepresentation final:public MediaFileSystems::ImageRepresentation,public RepresentationInfo {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit FluxRepresentation(std::shared_ptr<const Flux::ProtectedTrackImage>);
    const Flux::ProtectedTrackImage& image()const noexcept{return *image_;}
    const std::shared_ptr<const Flux::ProtectedTrackImage>& imageOwner()const noexcept{return image_;}
    std::span<const Flux::ProtectedTrack> tracks()const noexcept{return image_->tracks();}
    std::span<const std::int32_t> surfaces()const noexcept{return surfaces_;}
    RETROMEDIALAB_MEDIAENGINE_API std::int32_t revolutionCount()const;
    RETROMEDIALAB_MEDIAENGINE_API std::int64_t transitionCount()const;
    RepresentationKind representationKind()const noexcept override{return RepresentationKind::Flux;}
    std::optional<std::int64_t> logicalLength()const override{return {};}
    bool supportsRandomAccess()const override{return true;}
    bool supportsSequentialAccess()const override{return true;}
private:std::shared_ptr<const Flux::ProtectedTrackImage> image_;std::vector<std::int32_t> surfaces_;
};
}
