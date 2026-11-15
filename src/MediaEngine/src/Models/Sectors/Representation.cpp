#include "Representation.h"
#include "../../Sources/DriveAccess.h"
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
namespace {
std::int32_t coordinate(std::int64_t value){if(value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())throw M::MediaError(M::DiagnosticCode::BlockOverflow);return std::int32_t(value);}
DrivePosition position(M::SectorAddress address,std::uint64_t logical){
    if(address.cylinder>=0&&address.head>=0)return {coordinate(address.cylinder),coordinate(address.head),coordinate(address.number),-1};
    if(logical>std::uint64_t(std::numeric_limits<std::int64_t>::max()))throw M::MediaError(M::DiagnosticCode::BlockOverflow);return {0,0,-1,std::int64_t(logical)};
}
class DriveSectorImage final:public M::SectorImage {
public:
    DriveSectorImage(std::shared_ptr<const M::SectorImage> image,std::shared_ptr<DriveMechanics> drive,bool writing):image_(std::move(image)),drive_(std::move(drive)),writing_(writing){}
    const M::SectorGeometry& geometry()const noexcept override{return image_->geometry();}
    std::uint64_t blockCount()const noexcept override{return image_->blockCount();}
    std::uint64_t capacity()const noexcept override{return image_->capacity();}
    bool allowsVariableBlockSize()const noexcept override{return image_->allowsVariableBlockSize();}
    bool containsBlock(std::uint64_t logical)const noexcept override{return image_->containsBlock(logical);}
    std::optional<M::SectorAddress> blockAddress(std::uint64_t logical)const override{return image_->blockAddress(logical);}
    void visitAvailableBlocks(const std::function<bool(std::uint64_t)>& visitor,std::stop_token token)const override{image_->visitAvailableBlocks(visitor,token);}
    std::shared_ptr<const M::SectorBlock> block(std::uint64_t logical,std::stop_token token)const override{
        M::checkCancellation(token);const auto address=image_->blockAddress(logical);if(!address)return image_->block(logical,token);
        DriveAccessScope access(drive_.get(),position(*address,logical),writing_);const auto result=image_->block(logical,token);access.finish();return result;
    }
    const std::shared_ptr<DriveMechanics>& drive()const noexcept{return drive_;}
    bool writing()const noexcept{return writing_;}
private:std::shared_ptr<const M::SectorImage> image_;std::shared_ptr<DriveMechanics> drive_;bool writing_;
};
}
SectorRepresentation::SectorRepresentation(std::shared_ptr<const M::SectorImage> image,std::shared_ptr<DriveMechanics> drive,bool writing):image_(std::move(image)),drive_(std::move(drive)),writing_(writing){
    if(!image_)throw M::MediaError(M::DiagnosticCode::RegistryNullImage);
    if(image_->capacity()>std::uint64_t(std::numeric_limits<std::int64_t>::max()))throw M::MediaError(M::DiagnosticCode::BlockOverflow);
    if(drive_){const auto existing=std::dynamic_pointer_cast<const DriveSectorImage>(image_);if(!existing||existing->drive()!=drive_||existing->writing()!=writing_)image_=std::make_shared<DriveSectorImage>(image_,drive_,writing_);}
}
std::shared_ptr<const SectorRepresentation> bindSectors(std::shared_ptr<const SectorRepresentation> image,std::shared_ptr<DriveMechanics> drive,bool writing){if(!image)throw M::MediaError(M::DiagnosticCode::RegistryNullImage);if(!drive||(image->drive()==drive&&image->driveWriting()==writing))return image;return std::make_shared<SectorRepresentation>(image->imageOwner(),std::move(drive),writing);}
}
