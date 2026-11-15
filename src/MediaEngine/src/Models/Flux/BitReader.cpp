#include "BitReader.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Flux {
namespace {
bool range(BitView bits,std::int64_t offset,std::uint64_t length) noexcept{return offset>=0&&std::uint64_t(offset)<=bits.size()&&length<=bits.size()-std::uint64_t(offset);}
std::optional<std::uint8_t> byte(BitView bits,std::int64_t offset,std::int32_t stride) noexcept{
    if(!range(bits,offset,std::uint64_t(Constants::BitsPerByte)*stride))return {};
    std::uint8_t value=0;for(int bit=0;bit<Constants::BitsPerByte;++bit)if(bits[std::size_t(offset)+bit*stride+stride-1])value|=std::uint8_t(1<<(Constants::BitsPerByte-1-bit));return value;
}
}
bool match(BitView bits,std::int64_t offset,std::uint32_t pattern,std::int32_t length) noexcept{
    if(length<Constants::MinimumPatternBits||length>Constants::MaximumUintPatternBits||!range(bits,offset,length))return false;
    for(int bit=0;bit<length;++bit)if(bool(bits[std::size_t(offset)+bit])!=bool(pattern&(std::uint32_t(1)<<(length-1-bit))))return false;return true;
}
bool matchBytes(BitView bits,std::int64_t offset,std::span<const std::uint8_t> pattern) noexcept{
    if(pattern.size()>Constants::MaximumBits/Constants::BitsPerByte||!range(bits,offset,pattern.size()*Constants::BitsPerByte))return false;
    for(std::size_t index=0;index<pattern.size();++index)for(int bit=0;bit<Constants::BitsPerByte;++bit)if(bool(bits[std::size_t(offset)+index*Constants::BitsPerByte+bit])!=bool(pattern[index]&(1<<(Constants::BitsPerByte-1-bit))))return false;return true;
}
std::optional<std::uint8_t> mfmByte(BitView bits,std::int64_t offset) noexcept{return byte(bits,offset,Constants::MfmCellsPerDataBit);}
std::optional<std::uint8_t> rawByte(BitView bits,std::int64_t offset) noexcept{return byte(bits,offset,1);}
std::optional<std::uint8_t> fmByte32(BitView bits,std::int64_t offset) noexcept{return byte(bits,offset,Constants::FmCellsPerDataBit);}
std::optional<std::vector<std::uint8_t>> fmBytes32(BitView bits,std::int64_t offset,std::int64_t count,std::stop_token token){
    checkCancellation(token);
    const auto maximum=Constants::MaximumBits/Constants::BitsPerByte/Constants::FmCellsPerDataBit;
    if(count<0||std::uint64_t(count)>maximum)throw MediaError(DiagnosticCode::BitReadCount,{std::to_string(count),std::to_string(maximum)});
    if(count==0)return std::vector<std::uint8_t>{};
    const auto stride=Constants::BitsPerByte*Constants::FmCellsPerDataBit;
    if(!range(bits,offset,std::uint64_t(count)*stride))return {};
    std::vector<std::uint8_t> result;result.reserve(std::size_t(count));
    for(std::int64_t index=0;index<count;++index){checkCancellation(token);result.push_back(*fmByte32(bits,offset+index*stride));}
    return result;
}
std::optional<std::vector<std::uint8_t>> mfmBytes(BitView bits,std::int64_t offset,std::int64_t count,std::stop_token token){
    checkCancellation(token);const auto maximum=Constants::MaximumBits/Constants::BitsPerByte/Constants::MfmCellsPerDataBit;
    if(count<0||std::uint64_t(count)>maximum)throw MediaError(DiagnosticCode::BitReadCount,{std::to_string(count),std::to_string(maximum)});
    if(count==0)return std::vector<std::uint8_t>{};
    if(!range(bits,offset,std::uint64_t(count)*Constants::BitsPerByte*Constants::MfmCellsPerDataBit))return {};
    std::vector<std::uint8_t> result;result.reserve(std::size_t(count));
    for(std::int64_t index=0;index<count;++index){checkCancellation(token);result.push_back(*mfmByte(bits,offset+index*Constants::BitsPerByte*Constants::MfmCellsPerDataBit));}return result;
}
}
