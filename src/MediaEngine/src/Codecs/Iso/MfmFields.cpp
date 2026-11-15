#include "MfmInternal.h"
#include "../Crc16.h"
#include "../../Encoding/Bits.h"
#include "../../Models/Sectors/SizeCodes.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::IsoMfm::Internal {
using namespace Constants;
namespace {
void field(Flux::Bits& bits,MediaFileSystems::Bytes values,std::uint16_t crc,std::int32_t gap,std::stop_token token){
    checkCancellation(token);Encoding::raw(bits,EncodedSync);
    std::vector<std::uint8_t> bytes(values.begin(),values.end());bytes.push_back(std::uint8_t(crc>>Crc16::Constants::ByteBits));bytes.push_back(std::uint8_t(crc));
    Encoding::mfm(bits,bytes);Encoding::gap(bits,gap);
}
std::int32_t option(const Encoding::Request& request,std::string_view key){if(request.attributes){const auto found=request.attributes->find(std::string(key));if(found!=request.attributes->end())return found->second;}return 0;}
}
bool physical(const Encoding::Request& request){return option(request,PhysicalAttribute)==PhysicalEnabled;}
bool complete(const Encoding::Request& request){return option(request,CompleteAttribute)!=0;}
void validateAddress(std::string_view field,std::int32_t value){if(value<0||value>MaximumAddress)throw MediaError(DiagnosticCode::MfmAddressRange,{std::string(field),std::to_string(value),std::to_string(MaximumAddress)});}
std::uint8_t sizeCode(const Encoding::Sector& sector){
    if(sector.recordedSizeCode)return *sector.recordedSizeCode;
    const auto code=sector.sizeCode?*sector.sizeCode:sectorSizeCode(std::int64_t(sector.data.size()));
    const auto size=code<=MaximumSizeCode?sectorSizeBytes(code):0;
    if(size!=sector.data.size())throw MediaError(DiagnosticCode::MfmSizeCode,{std::to_string(code),std::to_string(size),std::to_string(sector.data.size())});
    return code;
}
void run(Flux::Bits& bits,std::uint8_t value,std::int32_t count,std::stop_token token){
    checkCancellation(token);const std::vector<std::uint8_t> bytes(std::size_t(count),value);Encoding::mfm(bits,bytes,!bits.empty()&&bits.back());
}
void address(Flux::Bits& bits,const Encoding::Request& request,const Encoding::Sector& sector,std::uint8_t code,std::int32_t gap,std::stop_token token){
    const auto cylinder=sector.recordedCylinder.value_or(request.cylinder),head=sector.recordedHead.value_or(request.head);
    validateAddress(RecordedCylinderField,cylinder);validateAddress(RecordedHeadField,head);
    const std::vector<std::uint8_t> bytes{SyncByte,SyncByte,SyncByte,IdMark,std::uint8_t(cylinder),std::uint8_t(head),std::uint8_t(sector.number),code};
    auto crc=Crc16::compute(bytes,Crc16::Constants::Ccitt,Crc16::Constants::AllBitsSet,token);if(sector.headerIntegrityValid==false)crc^=1;
    field(bits,MediaFileSystems::Bytes(bytes).subspan(SyncByteCount),crc,gap,token);
}
void data(Flux::Bits& bits,const Encoding::Sector& sector,std::int32_t gap,std::stop_token token){
    std::vector<std::uint8_t> bytes(SyncByteCount,SyncByte);bytes.push_back(sector.deleted?DeletedMark:DataMark);bytes.insert(bytes.end(),sector.data.begin(),sector.data.end());
    const auto recorded=sector.recordedErrorCheckCode.size()==CrcBytes;
    auto crc=recorded?std::uint16_t((sector.recordedErrorCheckCode[0]<<Crc16::Constants::ByteBits)|sector.recordedErrorCheckCode[1]):Crc16::compute(bytes,Crc16::Constants::Ccitt,Crc16::Constants::AllBitsSet,token);
    if(!recorded&&sector.dataIntegrityValid==false)crc^=1;
    field(bits,MediaFileSystems::Bytes(bytes).subspan(SyncByteCount),crc,gap,token);
}
}
