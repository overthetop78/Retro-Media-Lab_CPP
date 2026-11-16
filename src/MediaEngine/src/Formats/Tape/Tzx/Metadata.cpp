#include "Metadata.h"
#include "Constants.h"
#include <QString>
#include <charconv>
#include <QtEndian>
namespace RetroMediaLab::MediaEngine::Tzx {
using namespace Constants;
std::string blockIdText(std::uint8_t id){return QString::number(id,HexRadix).rightJustified(HexWidth,QLatin1Char(DigitZero)).toStdString();}
bool tryBlockId(const SequentialSegmentFields& value,std::uint8_t& id){
    const auto found=value.metadata.find(std::string(BlockIdMetadataKey));if(found==value.metadata.end())return false;
    const auto text=QString::fromStdString(found->second).trimmed().toStdString();unsigned parsed{};const auto result=std::from_chars(text.data(),text.data()+text.size(),parsed,HexRadix);
    if(result.ec!=std::errc{}||result.ptr!=text.data()+text.size()||parsed>MaximumByte)return false;id=static_cast<std::uint8_t>(parsed);return true;
}
bool tryDataRange(const SequentialSegmentFields& value,std::span<const std::uint8_t> payload,int& offset,int& length){
    offset=length=0;std::uint8_t id{};if(!tryBlockId(value,id))return false;
    const auto triple=[&](int at){return int(payload[at])|(int(payload[at+1])<<ByteBits)|(int(payload[at+WordWidth])<<(WordWidth*ByteBits));};
    switch(id){
    case StandardSpeedData:offset=StandardHeaderSize;if(payload.size()<offset)return false;length=qFromLittleEndian<std::uint16_t>(payload.data()+StandardLengthOffset);break;
    case TurboSpeedData:offset=TurboHeaderSize;if(payload.size()<offset)return false;length=triple(TurboLengthOffset);break;
    case PureData:offset=PureHeaderSize;if(payload.size()<offset)return false;length=triple(PureLengthOffset);break;
    default:return false;
    }
    return length>0&&static_cast<std::size_t>(length)<=payload.size()-offset;
}
}
