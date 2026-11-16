#include "Decoder.h"
#include "../../../Formats/Tape/AtariCas/Constants.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AtariTape {
using namespace AtariTapeConstants;
std::uint8_t checksum(std::span<const std::uint8_t> bytes){unsigned result=0;for(auto value:bytes){result+=value;result=(result&ChecksumMask)+(result>>ChecksumShift);}return std::uint8_t(result);}
std::optional<Record> standardRecord(std::span<const std::uint8_t> bytes) {
    if(bytes.size()<MinimumRecordLength||bytes[0]!=RecordSync||bytes[1]!=RecordSync)return {};const auto type=bytes[WordBytes];
    if(type!=FullRecord&&type!=PartialRecord&&type!=EndRecord)return {};if(type==EndRecord)return Record{type,{},checksum(bytes.first(bytes.size()-1))==bytes.back()};
    if(bytes.size()!=StandardRecordLength)return {};const auto count=type==FullRecord?RecordDataLength:std::min<int>(bytes[PartialLengthOffset],RecordDataLength);
    return Record{type,bytes.subspan(RecordDataOffset,count),checksum(bytes.first(ChecksumOffset))==bytes[ChecksumOffset]};
}
bool extendedPadding(std::span<const std::uint8_t> bytes) {
    return bytes.size()>StandardRecordLength&&bytes[0]==RecordSync&&bytes[1]==RecordSync&&bytes[WordBytes]==FullRecord&&std::all_of(bytes.begin()+RecordDataOffset,bytes.end()-1,[](auto v){return v==0;})&&checksum(bytes.first(bytes.size()-1))==bytes.back();
}
}
