#include "BitPacker.h"
#include "PackingConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Encoding {
std::size_t requiredByteCount(std::int32_t bitCount){
    if(bitCount<=0)throw MediaError(DiagnosticCode::BitPackingCount,{std::to_string(bitCount)});
    if(bitCount>PackingConstants::MaximumBitCount-std::int32_t(PackingConstants::BitsPerByte))throw MediaError(DiagnosticCode::BitPackingRange);
    return (std::size_t(bitCount)+PackingConstants::RoundUp)/PackingConstants::BitsPerByte;
}
void packBits(Flux::BitView bits,MediaFileSystems::MutableBytes destination,bool padding){
    using namespace PackingConstants;
    if(bits.size()>std::size_t(MaximumBitCount))throw MediaError(DiagnosticCode::BitPackingRange);
    if((bits.size()+RoundUp)/BitsPerByte>destination.size())throw MediaError(DiagnosticCode::BitPackingCapacity,{std::to_string(bits.size()),std::to_string(destination.size())});
    std::fill(destination.begin(),destination.end(),padding?OneFill:ZeroFill);
    for(std::size_t bit=0;bit<bits.size();++bit){const auto mask=std::uint8_t(1<<(BitsPerByte-1-bit%BitsPerByte));if(bits[bit])destination[bit/BitsPerByte]|=mask;else destination[bit/BitsPerByte]&=std::uint8_t(~mask);}
}
std::vector<std::uint8_t> packBits(Flux::BitView bits,bool padding){
    if(bits.size()>std::size_t(PackingConstants::MaximumBitCount))throw MediaError(DiagnosticCode::BitPackingRange);
    std::vector<std::uint8_t> bytes(bits.empty()?0:requiredByteCount(std::int32_t(bits.size())));packBits(bits,bytes,padding);return bytes;
}
Flux::Bits unpackBits(MediaFileSystems::Bytes bytes,std::int32_t bitCount){
    const auto needed=requiredByteCount(bitCount);if(needed>bytes.size())throw MediaError(DiagnosticCode::BitPackingCapacity,{std::to_string(bitCount),std::to_string(bytes.size())});
    Flux::Bits bits(std::size_t(bitCount),0);
    for(std::size_t bit=0;bit<bits.size();++bit)bits[bit]=std::uint8_t((bytes[bit/PackingConstants::BitsPerByte]>>(PackingConstants::BitsPerByte-1-bit%PackingConstants::BitsPerByte))&1);
    return bits;
}
}
