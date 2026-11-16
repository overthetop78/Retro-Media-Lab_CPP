#include <RetroMediaLab/MediaFileSystems/FileCore.h>
#include "Constants.h"
#include "../../../Primitives/Binary.h"
#include <bit>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::FileCore {
using namespace Constants;
bool validChecksum(Bytes bytes) {
    if(bytes.empty())return false;
    std::uint32_t sum=0;
    for(auto offset=std::int64_t(bytes.size())-PairCount;offset>=0;--offset)sum=(sum&ByteMask)+(sum>>BitsPerByte)+bytes[std::size_t(offset)];
    return std::uint8_t(sum)==bytes.back();
}
std::optional<std::uint64_t> oldMapDeclaredCapacity(Bytes bytes) {
    if(bytes.size()<PairCount*FileCoreUnitSize||!validChecksum(bytes.first(FileCoreUnitSize))||!validChecksum(bytes.subspan(FileCoreUnitSize,FileCoreUnitSize)))return {};
    const auto units=std::uint64_t(Binary::little24(bytes,TotalUnitsOffset));
    return units*FileCoreUnitSize;
}
std::int32_t DiscRecord::zoneSizeBits() const {
    const auto bits=std::int64_t(applyShift(BitsPerByte,log2SectorSize))-zoneSpareBits;
    if(bits<std::numeric_limits<std::int32_t>::min()||bits>std::numeric_limits<std::int32_t>::max())throw MediaError(DiagnosticCode::FileCoreOverflow);
    return std::int32_t(bits);
}
Address decodeAddress(std::int32_t value) noexcept { return {std::uint32_t(value) >> BitsPerByte, std::uint32_t(value) & ByteMask}; }
std::string decodeDiscName(Bytes data) {
    std::string result;result.reserve(data.size());
    for(auto value:data)result.push_back(value>SevenBitMask?AsciiReplacement:char(value));
    while(!result.empty()&&(result.back()==0||result.back()==CarriageReturn||result.back()==Space))result.pop_back();
    return result;
}
std::string decodeAdfsName(Bytes data) {
    std::string result;result.reserve(data.size());
    for(auto value:data){const auto character=char(value&SevenBitMask);if(character==0||character==CarriageReturn)break;result.push_back(character);}
    while(!result.empty()&&result.back()==Space)result.pop_back();return result;
}
std::optional<DiscRecord> parseDiscRecord(Bytes data,std::uint64_t capacity) {
    if(data.size()<RecordLength)return {};
    const auto sector=data[0],id=data[IdLengthOffset],mapBit=data[MapBitOffset];
    const auto zones=int(data[ZonesLowOffset])|(int(data[ZonesHighOffset])<<BitsPerByte);
    const auto spare=Binary::little<std::uint16_t>(data,ZoneSpareOffset);
    const auto root=Binary::little<std::uint32_t>(data,RootOffset);
    const auto size=std::uint64_t(Binary::little<std::uint32_t>(data,SizeLowOffset))|(std::uint64_t(Binary::little<std::uint32_t>(data,SizeHighOffset))<<WordBits);
    if(sector<MinLogSector||sector>MaxLogSector||id<sector+MinExtraIdBits||id>MaxIdBits||mapBit>sector||zones<1||root<1||root>std::uint32_t(std::numeric_limits<std::int32_t>::max())||size<1||size>capacity||size>std::uint64_t(std::numeric_limits<std::int64_t>::max())||spare>=(BitsPerByte<<sector))return {};
    return DiscRecord{sector,id,mapBit,zones,spare,std::int32_t(root),size,decodeDiscName(data.subspan(NameOffset,NameLength)),data[ShareOffset]&ShareMask};
}
std::uint32_t getBits(Bytes data,std::int64_t offset,std::uint32_t mask) {
    const auto length=std::max(1,int(std::bit_width(mask)));
    const auto capacity=std::uint64_t(data.size())*BitsPerByte;
    if(offset<0||std::uint64_t(offset)>capacity||std::uint64_t(length)>capacity-std::uint64_t(offset))throw MediaError(DiagnosticCode::FileCoreBitRange,{std::to_string(offset),std::to_string(length),std::to_string(capacity)});
    const auto byteOffset=std::size_t(offset/BitsPerByte);
    const auto intra=int(offset%BitsPerByte);
    const auto count=(intra+length+BitsPerByte-1)/BitsPerByte;
    std::uint64_t window=0;
    for(int byte=0;byte<count;++byte)window|=std::uint64_t(data[byteOffset+byte])<<(byte*BitsPerByte);
    return std::uint32_t(window>>intra)&mask;
}
std::int64_t findNextSetBit(Bytes data,std::int64_t start,std::int64_t end) {
    const auto capacity=std::uint64_t(data.size())*BitsPerByte;
    if(start<0||end<start||std::uint64_t(end)>capacity)throw MediaError(DiagnosticCode::FileCoreBitRange,{std::to_string(start),std::to_string(end),std::to_string(capacity)});
    for(auto bit=start;bit<end;++bit)if((data[std::size_t(bit/BitsPerByte)]>>(bit%BitsPerByte))&1)return bit;
    return end;
}
std::int32_t applyShift(std::int32_t value,std::int32_t shift) {
    if(shift<=-WordBits||shift>=WordBits)throw MediaError(DiagnosticCode::FileCoreShift,{std::to_string(shift)});
    if(shift<0)return value>>-shift;
    const auto shifted=std::int64_t(value)*(std::int64_t(1)<<shift);
    if(shifted<std::numeric_limits<std::int32_t>::min()||shifted>std::numeric_limits<std::int32_t>::max())throw MediaError(DiagnosticCode::FileCoreOverflow);
    return std::int32_t(shifted);
}
}
