#include "Commodore1541.h"
#include "Gcr.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <bit>
namespace RetroMediaLab::MediaEngine::Commodore1541Gcr {
std::int32_t diskTrack(std::int32_t cylinder,std::int32_t head) noexcept{return std::bit_cast<std::int32_t>(std::uint32_t(cylinder)+Constants::MinimumDiskTrack+std::uint32_t(head)*Constants::TracksPerSide);}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
    using namespace Constants;
    const auto readByte=[&](std::string_view key,std::uint8_t fallback){const auto value=attribute(request.attributes,std::string(key),fallback);if(value<0||value>MaximumByte)throw MediaError(DiagnosticCode::CbmGcrAttributeRange,{std::string(key),std::to_string(value),std::to_string(MaximumByte)});return std::uint8_t(value);};
    const auto id2=readByte(Id2Attribute,DefaultId2),id1=readByte(Id1Attribute,DefaultId1);
    if(request.cylinder>MaximumCylinder)throw MediaError(DiagnosticCode::TrackCylinder,{std::to_string(request.cylinder),std::to_string(MaximumCylinder)});
    const auto tracks=attribute(request.attributes,std::string(TracksPerSideAttribute),TracksPerSide);
    const auto fallback=std::bit_cast<std::int32_t>(std::uint32_t(request.cylinder)+MinimumDiskTrack+std::uint32_t(request.head)*std::uint32_t(tracks));
    const auto track=attribute(request.attributes,std::string(TrackAttribute),fallback);
    if(track<MinimumDiskTrack||track>MaximumDiskTrack)throw MediaError(DiagnosticCode::CbmGcrDiskTrack,{std::to_string(track),std::to_string(MinimumDiskTrack),std::to_string(MaximumDiskTrack)});
    Flux::Bits bits;
    const auto append=[&](MediaFileSystems::Bytes record){const auto encoded=CommodoreGcr::encode(record,token);if(encoded.size()>MaximumBits-bits.size())throw MediaError(DiagnosticCode::FluxSizeLimit);bits.insert(bits.end(),encoded.begin(),encoded.end());};
    for(const auto& sector:request.sectors){
        checkCancellation(token);if(sector.data.size()!=SectorBytes)throw MediaError(DiagnosticCode::CbmGcrSectorSize,{std::to_string(SectorBytes),std::to_string(sector.data.size())});
        if(sector.number<0||sector.number>MaximumByte)throw MediaError(DiagnosticCode::TrackSectorNumber,{std::to_string(sector.number),std::to_string(MaximumByte)});
        Encoding::gap(bits,LeadingGap,true);if(RawGap>MaximumBits-bits.size())throw MediaError(DiagnosticCode::FluxSizeLimit);bits.insert(bits.end(),RawGap,0);Encoding::gap(bits,SyncGap,true);
        const std::vector<std::uint8_t> header{HeaderMark,std::uint8_t(sector.number^track^id2^id1),std::uint8_t(sector.number),std::uint8_t(track),id2,id1};append(header);
        Encoding::gap(bits,HeaderDataGap);Encoding::gap(bits,SyncGap,true);
        std::vector<std::uint8_t> data;data.reserve(DataBytes);data.push_back(DataMark);data.insert(data.end(),sector.data.begin(),sector.data.end());data.push_back(CommodoreGcr::checksum(sector.data));append(data);
        Encoding::gap(bits,TrailingGap);
    }
    return bits;
}
}
