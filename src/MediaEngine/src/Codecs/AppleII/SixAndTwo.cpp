#include "Internal.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AppleII {
std::vector<std::uint8_t> encodeSixAndTwo(MediaFileSystems::Bytes source,std::stop_token token){
    using namespace Constants;checkCancellation(token);Internal::validateSize(source);
    std::array<std::uint8_t,MinimumSixAndTwoWorkBufferByteCount> buffer{};std::copy(source.begin(),source.end(),buffer.begin());
    std::vector<std::uint8_t> encoded;encoded.reserve(SixAndTwoEncodedByteCount);std::uint8_t checksum=0;
    for(int index=0;index<SixAndTwoAuxiliaryByteCount;++index){
        checkCancellation(token);const auto a=buffer[index],b=buffer[index+SixAndTwoAuxiliaryByteCount],c=buffer[index+SixAndTwoAuxiliaryByteCount*TwoBitShift];
        const auto value=std::uint8_t(((a&1)<<1)|((a&BitOneMask)>>1)|((b&1)<<ThreeBitShift)|((b&BitOneMask)<<1)|((c&1)<<FiveBitShift)|((c&BitOneMask)<<ThreeBitShift));
        encoded.push_back(SixAndTwoTable[value^checksum]);checksum=value;
    }
    for(int index=0;index<SectorSize;++index){checkCancellation(token);const auto value=std::uint8_t(source[index]>>TwoBitShift);encoded.push_back(SixAndTwoTable[value^checksum]);checksum=value;}
    encoded.push_back(SixAndTwoTable[checksum]);return encoded;
}
std::optional<DecodedBlock> decodeSixAndTwo(Flux::BitView bits,std::int64_t offset,std::stop_token token){
    using namespace Constants;checkCancellation(token);auto cursor=offset;auto values=Internal::values(bits,cursor,SixAndTwoEncodedByteCount,Internal::InverseSixAndTwoTable,token);if(!values)return std::nullopt;
    std::array<std::uint8_t,SixAndTwoDecodedByteCount> decoded{};std::uint8_t previous=0;std::size_t encodedIndex=0;
    for(int index=SixAndTwoDecodedByteCount-1;index>=SectorSize;--index){checkCancellation(token);decoded[index]=std::uint8_t((*values)[encodedIndex++]^previous);previous=decoded[index];}
    for(int index=0;index<SectorSize;++index){checkCancellation(token);decoded[index]=std::uint8_t((*values)[encodedIndex++]^previous);previous=decoded[index];}
    const bool valid=std::uint8_t((*values)[SixAndTwoEncodedByteCount-1]^previous)==0;
    std::vector<std::uint8_t> data(SectorSize);std::uint8_t auxiliaryOffset=0;
    for(int index=0;index<SectorSize;++index){
        checkCancellation(token);auxiliaryOffset=std::uint8_t((auxiliaryOffset+SixAndTwoAuxiliaryByteCount-1)%SixAndTwoAuxiliaryByteCount);
        const auto auxiliary=decoded[SectorSize+auxiliaryOffset];decoded[SectorSize+auxiliaryOffset]=std::uint8_t(auxiliary>>TwoBitShift);
        data[index]=std::uint8_t((decoded[index]<<TwoBitShift)|((auxiliary&BitOneMask)>>1)|((auxiliary&1)<<1));
    }
    return DecodedBlock{std::move(data),valid,cursor};
}
}
