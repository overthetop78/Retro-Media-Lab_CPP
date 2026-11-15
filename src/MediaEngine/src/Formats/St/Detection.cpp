#include "Geometry.h"
#include "Constants.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaFileSystems/FatBootSectorLayout.h>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::St {
namespace M=MediaFileSystems;
namespace {
template<class T> T little(M::Bytes bytes,std::size_t offset){T value=0;for(std::size_t index=0;index<sizeof(T);++index)value|=T(T(bytes[offset+index])<<(index*std::numeric_limits<std::uint8_t>::digits));return value;}
std::optional<M::SectorGeometry> bpb(M::Bytes data){
    namespace B=M::FatBootSectorLayout;
    if(data.size()<B::MinimumLength)return {};
    const auto size=little<std::uint16_t>(data,B::BytesPerSectorOffset);auto total=little<std::uint16_t>(data,B::TotalSectors16Offset);
    if(!total)total=std::uint16_t(std::min(std::uint32_t(std::numeric_limits<std::uint16_t>::max()),little<std::uint32_t>(data,B::TotalSectors32Offset)));
    const auto sectors=little<std::uint16_t>(data,B::SectorsPerTrackOffset),heads=little<std::uint16_t>(data,B::HeadCountOffset);
    if(size!=Constants::SectorSize||total!=data.size()/Constants::SectorSize||!sectors||sectors>Constants::MaximumBpbSectors||!heads||heads>B::MaximumHeadCount||total%(std::uint32_t(sectors)*heads))return {};
    return geometry(total/(std::uint32_t(sectors)*heads),heads,sectors);
}
}
GeometryDetection detect(M::Bytes data){
    if(const auto value=bpb(data))return {*value,GeometryEvidence::Bpb};const auto count=data.size()/Constants::SectorSize;
    for(const auto sectors:Constants::FallbackSectors)for(const auto heads:Constants::FallbackHeads){const auto unit=sectors*heads;const auto cylinders=count/unit;if(count%unit==0&&cylinders>=Constants::MinimumFallbackCylinders&&cylinders<=Constants::MaximumFallbackCylinders)return {geometry(std::int32_t(cylinders),std::int32_t(heads),std::int32_t(sectors)),GeometryEvidence::CapacityFallback};}
    throw M::MediaError(M::DiagnosticCode::StRawGeometry,{std::to_string(data.size())});
}
std::shared_ptr<const M::SectorImage> read(M::Bytes data,std::stop_token token){
    if(data.empty()||data.size()%Constants::SectorSize)throw M::MediaError(M::DiagnosticCode::StRawLength,{std::to_string(data.size()),std::to_string(Constants::SectorSize)});
    const auto detected=detect(data);const auto& dimensions=detected.geometry;const auto capacity=std::uint64_t(dimensions.cylinders)*dimensions.heads*dimensions.sectorsPerTrack*dimensions.blockSize;
    if(data.size()!=capacity)throw M::MediaError(M::DiagnosticCode::StRawCapacity,{std::to_string(data.size()),std::to_string(capacity),std::to_string(dimensions.cylinders),std::to_string(dimensions.heads),std::to_string(dimensions.sectorsPerTrack)});
    M::checkCancellation(token);SourceSectorMetadata metadata;metadata.trustCompletePayload=true;return sourceSectorImage(memorySource(data),dimensions,0,{},metadata);
}
}
