#include "Mfm.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AmigaMfm {
namespace {
using namespace Constants;
void validate(std::string_view field,std::int32_t value,std::int32_t maximum){
    if(value<0||value>maximum)throw MediaError(DiagnosticCode::AmigaMfmAddressRange,{std::string(field),std::to_string(value),std::to_string(maximum)});
}
std::vector<std::uint8_t> buildSector(const Encoding::Request& request,const Encoding::Sector& sector,std::size_t remaining,std::stop_token token){
    const std::array<std::uint8_t,InfoByteCount> info{FormatByte,packTrack(request.cylinder,request.head),std::uint8_t(sector.number),std::uint8_t(remaining)};
    auto header=encodeOddEven(info,token);header.resize(HeaderParitySourceByteCount);
    const auto headerParity=parity(header,0,std::int64_t(header.size()),token);
    const auto data=encodeOddEven(sector.data,token);
    const auto dataParity=splitParity(data,0,std::int64_t(data.size()),token);
    std::vector<std::uint8_t> encoded(EncodedSectorByteCount);
    std::copy(header.begin(),header.end(),encoded.begin());
    encoded[HeaderParityHighOffset]=headerParity.first;encoded[HeaderParityLowOffset]=headerParity.second;
    encoded[DataParityHighOffset]=dataParity.first;encoded[DataParityLowOffset]=dataParity.second;
    std::copy(data.begin(),data.end(),encoded.begin()+EncodedDataOffset);
    return encoded;
}
}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
    validate(Encoding::Constants::CylinderField,request.cylinder,Constants::MaximumCylinder);
    validate(Encoding::Constants::HeadField,request.head,Constants::MaximumHead);
    if(request.sectors.size()>Constants::MaximumRemainingSectorCount)throw MediaError(DiagnosticCode::AmigaMfmAddressRange,{std::string(Constants::SectorCountField),std::to_string(request.sectors.size()),std::to_string(Constants::MaximumRemainingSectorCount)});
    Flux::Bits bits;
    for(std::size_t index=0;index<request.sectors.size();++index){
        checkCancellation(token);const auto& sector=request.sectors[index];
        if(sector.data.size()!=Constants::SectorByteCount)throw MediaError(DiagnosticCode::AmigaMfmSectorSize,{std::to_string(sector.data.size())});
        validate(Constants::SectorNumberField,sector.number,Constants::MaximumSectorNumber);
        const auto encoded=buildSector(request,sector,request.sectors.size()-index,token);
        Encoding::gap(bits,Constants::LeadingGapBitCount);Encoding::raw(bits,Constants::SyncBytes);
        Encoding::mfm(bits,encoded);Encoding::gap(bits,Constants::TrailingGapBitCount);
    }
    return bits;
}
}
