#pragma once
#include <RetroMediaLab/MediaEngine/RepresentationInfo.h>
#include <RetroMediaLab/MediaEngine/DriveMechanics.h>
#include <RetroMediaLab/MediaFileSystems/Representations.h>
namespace RetroMediaLab::MediaEngine {
class SectorRepresentation final:public MediaFileSystems::SectorRepresentation,public RepresentationInfo {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit SectorRepresentation(std::shared_ptr<const MediaFileSystems::SectorImage>,std::shared_ptr<DriveMechanics> drive={},bool writing=false);
    const MediaFileSystems::SectorImage& image()const override{return *image_;}
    const std::shared_ptr<const MediaFileSystems::SectorImage>& imageOwner()const noexcept{return image_;}
    const std::shared_ptr<DriveMechanics>& drive()const noexcept{return drive_;}
    bool driveWriting()const noexcept{return writing_;}
    RepresentationKind representationKind()const noexcept override{return RepresentationKind::Sectors;}
    std::optional<std::int64_t> logicalLength()const override{return std::int64_t(image_->capacity());}
    bool supportsRandomAccess()const override{return true;}
    bool supportsSequentialAccess()const override{return true;}
private:std::shared_ptr<const MediaFileSystems::SectorImage> image_;std::shared_ptr<DriveMechanics> drive_;bool writing_;
};
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const SectorRepresentation> bindSectors(std::shared_ptr<const SectorRepresentation>,std::shared_ptr<DriveMechanics>,bool writing);
}
