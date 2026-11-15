#include "Image.h"
#include "Internal.h"
#include "../../Sources/DriveRandomAccessData.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
namespace {
bool sameDrive(const std::shared_ptr<const M::RandomAccessData>& data,const std::shared_ptr<DriveMechanics>& drive,bool writing){const auto wrapper=std::dynamic_pointer_cast<const DriveRandomAccessData>(data);return wrapper&&wrapper->drive()==drive&&wrapper->writing()==writing;}
std::shared_ptr<const M::RandomAccessData> bind(std::shared_ptr<const M::RandomAccessData> data,std::shared_ptr<DriveMechanics> drive,std::int64_t firstSector,std::int64_t sourceOffset,std::int32_t sectorSize,bool writing){
    return std::make_shared<DriveRandomAccessData>(std::move(data),std::move(drive),[firstSector,sourceOffset,sectorSize](std::int64_t offset){return DrivePosition{0,0,-1,OpticalInternal::add(firstSector,(offset-sourceOffset)/sectorSize)};},[sourceOffset,sectorSize](std::int64_t offset){return sectorSize-(offset-sourceOffset)%sectorSize;},writing);
}
}
std::shared_ptr<const OpticalImage> bindOptical(std::shared_ptr<const OpticalImage> image,std::shared_ptr<DriveMechanics> drive,bool writing){
    if(!image)throw M::MediaError(M::DiagnosticCode::DocumentRepresentation);if(!drive||!image->options().tracks)return image;
    if(std::all_of(image->options().tracks->begin(),image->options().tracks->end(),[&](const auto& track){return sameDrive(track->options().dataSource,drive,writing);}))return image;
    auto options=image->options();for(auto& track:*options.tracks){auto value=track->options();value.dataSource=bind(value.dataSource,drive,value.firstSector,value.sourceOffset,value.storedSectorSize,writing);if(value.subchannelSource)value.subchannelSource=bind(value.subchannelSource,drive,value.firstSector,value.subchannelSourceOffset,value.subchannelStride,writing);track=std::make_shared<OpticalTrack>(std::move(value));}
    return std::make_shared<OpticalImage>(std::move(options));
}
}
