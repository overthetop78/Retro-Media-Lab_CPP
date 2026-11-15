#include "Internal.h"
namespace RetroMediaLab::MediaEngine::AppleII {
std::vector<std::uint8_t> encodeFiveAndThree(MediaFileSystems::Bytes source,std::stop_token token){
    using namespace Constants;checkCancellation(token);Internal::validateSize(source);
    std::array<std::uint8_t,SectorSize> top{};std::array<std::uint8_t,FiveAndThreeAuxiliaryByteCount> threes{};int chunk=FiveAndThreeChunkByteCount-1;std::size_t sourceOffset=0;
    for(int index=0;index<FiveAndThreeChunkByteCount*FiveGroupCount;index+=FiveGroupCount){
        checkCancellation(token);const auto zero=source[sourceOffset++],one=source[sourceOffset++],two=source[sourceOffset++],three=source[sourceOffset++],four=source[sourceOffset++];
        top[chunk]=std::uint8_t(zero>>ThreeBitShift);top[chunk+FiveAndThreeChunkByteCount]=std::uint8_t(one>>ThreeBitShift);top[chunk+FiveAndThreeChunkByteCount*TwoBitShift]=std::uint8_t(two>>ThreeBitShift);top[chunk+FiveAndThreeChunkByteCount*ThreeBitShift]=std::uint8_t(three>>ThreeBitShift);top[chunk+FiveAndThreeChunkByteCount*FourBitShift]=std::uint8_t(four>>ThreeBitShift);
        threes[chunk]=std::uint8_t(((zero&LowThreeMask)<<TwoBitShift)|((three&BitTwoMask)>>1)|((four&BitTwoMask)>>TwoBitShift));
        threes[chunk+FiveAndThreeChunkByteCount]=std::uint8_t(((one&LowThreeMask)<<TwoBitShift)|(three&BitOneMask)|((four&BitOneMask)>>1));
        threes[chunk+FiveAndThreeChunkByteCount*TwoBitShift]=std::uint8_t(((two&LowThreeMask)<<TwoBitShift)|((three&1)<<1)|(four&1));--chunk;
    }
    const auto last=source[sourceOffset];top[SectorSize-1]=std::uint8_t(last>>ThreeBitShift);threes.back()=std::uint8_t(last&LowThreeMask);
    std::vector<std::uint8_t> encoded;encoded.reserve(FiveAndThreeEncodedByteCount);std::uint8_t checksum=0;
    for(int index=FiveAndThreeAuxiliaryByteCount-1;index>=0;--index){checkCancellation(token);encoded.push_back(FiveAndThreeTable[threes[index]^checksum]);checksum=threes[index];}
    for(auto value:top){checkCancellation(token);encoded.push_back(FiveAndThreeTable[value^checksum]);checksum=value;}
    encoded.push_back(FiveAndThreeTable[checksum]);return encoded;
}
std::optional<DecodedBlock> decodeFiveAndThree(Flux::BitView bits,std::int64_t offset,std::stop_token token){
    using namespace Constants;checkCancellation(token);auto cursor=offset;auto values=Internal::values(bits,cursor,FiveAndThreeEncodedByteCount,Internal::InverseFiveAndThreeTable,token);if(!values)return std::nullopt;
    std::array<std::uint8_t,FiveAndThreeAuxiliaryByteCount> threes{};std::array<std::uint8_t,SectorSize> bases{};std::uint8_t checksum=0;std::size_t source=0;
    for(int index=FiveAndThreeAuxiliaryByteCount-1;index>=0;--index){checkCancellation(token);checksum^=(*values)[source++];threes[index]=checksum;}
    for(auto& value:bases){checkCancellation(token);checksum^=(*values)[source++];value=std::uint8_t(checksum<<ThreeBitShift);}
    const bool valid=(*values)[source]==checksum;std::vector<std::uint8_t> data(SectorSize);std::size_t destination=0;
    for(int index=FiveAndThreeChunkByteCount-1;index>=0;--index){
        checkCancellation(token);const auto one=threes[index],two=threes[FiveAndThreeChunkByteCount+index],three=threes[FiveAndThreeChunkByteCount*TwoBitShift+index];
        const auto four=std::uint8_t(((one&BitOneMask)<<1)|(two&BitOneMask)|((three&BitOneMask)>>1));
        const auto five=std::uint8_t(((one&1)<<TwoBitShift)|((two&1)<<1)|(three&1));
        data[destination++]=std::uint8_t(bases[index]|((one>>TwoBitShift)&LowThreeMask));
        data[destination++]=std::uint8_t(bases[FiveAndThreeChunkByteCount+index]|((two>>TwoBitShift)&LowThreeMask));
        data[destination++]=std::uint8_t(bases[FiveAndThreeChunkByteCount*TwoBitShift+index]|((three>>TwoBitShift)&LowThreeMask));
        data[destination++]=std::uint8_t(bases[FiveAndThreeChunkByteCount*ThreeBitShift+index]|(four&LowThreeMask));
        data[destination++]=std::uint8_t(bases[FiveAndThreeChunkByteCount*FourBitShift+index]|(five&LowThreeMask));
    }
    data[destination]=std::uint8_t(bases[SectorSize-1]|(threes.back()&LowThreeMask));return DecodedBlock{std::move(data),valid,cursor};
}
}
