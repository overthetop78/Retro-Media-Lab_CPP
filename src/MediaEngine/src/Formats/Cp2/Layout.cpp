#include "Cp2.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Cp2 {
std::vector<SectorDescriptor> parseTrack(std::span<const std::uint8_t> descriptor) {
    using namespace Constants;
    if(descriptor.size()<TrackDescriptorSize)throw MediaError(DiagnosticCode::Cp2DescriptionTruncated,{std::to_string(0),std::to_string(TrackDescriptorSize),std::to_string(descriptor.size())});
    const auto count=descriptor[TrackSectorCountOffset];if(count==0)return {};
    if(count>MaximumDescriptors||TrackHeaderSize+count*SectorDescriptorSize>descriptor.size())throw MediaError(DiagnosticCode::Cp2DescriptorCount,{std::to_string(count),std::to_string(MaximumDescriptors)});
    const auto cylinder=descriptor[TrackCylinderOffset],head=descriptor[TrackHeadOffset];std::vector<SectorDescriptor> sectors;sectors.reserve(count);
    for(std::size_t index=0;index<count;++index){const auto entry=descriptor.subspan(TrackHeaderSize+index*SectorDescriptorSize,SectorDescriptorSize);const auto code=entry[SectorSizeCodeOffset];
        if(entry[SectorCylinderOffset]!=cylinder||entry[SectorHeadOffset]!=head||code>MaximumSizeCode)continue;
        sectors.push_back({entry[SectorCylinderOffset],entry[SectorHeadOffset],entry[SectorNumberOffset],BaseSectorSize<<code,std::uint16_t(entry[SectorPositionOffset]|std::uint32_t(entry[SectorPositionOffset+1])<<ByteBits)});
    }return sectors;
}
}
