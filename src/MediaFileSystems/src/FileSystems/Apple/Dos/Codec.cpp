#include "Internal.h"
#include "../../../Primitives/Identifiers.h"
#include <QString>
namespace RetroMediaLab::MediaFileSystems::AppleDos {
using namespace Constants;
bool Internal::sameId(std::string_view a,std::string_view b) noexcept {
    if(a.size()!=b.size())return false;
    const auto upper=[](char c){return c>=LowerFirst&&c<=LowerLast?char(c-CaseDelta):c;};
    for(std::size_t i=0;i<a.size();++i)if(upper(a[i])!=upper(b[i]))return false;
    return true;
}
bool Internal::validAddress(std::int64_t track,std::int64_t sector,std::uint32_t tracks,std::uint32_t sectors) noexcept{return track>=0&&track<tracks&&sector>=0&&sector<sectors;}
bool validName(std::string_view value){
    const auto name=Identifiers::utf8(value);
    if(name.isEmpty()||name.size()>NameLength||name.front().unicode()<NameFirst||name.front().unicode()>PrintableLast||name.back().unicode()==Space)return false;
    for(auto c:name)if(c.unicode()<Space||c.unicode()>PrintableLast||c.unicode()==Comma)return false;
    return true;
}
std::optional<std::uint8_t> volumeNumber(std::string_view value){
    const auto name=Identifiers::utf8(value);
    if(name.size()!=VolumeTextLength||!Internal::sameId(name.first(qsizetype(VolumePrefix.size())).toUtf8().toStdString(),VolumePrefix))return {};
    auto number=name.sliced(qsizetype(VolumePrefix.size()));
    while(!number.isEmpty()&&number.back().unicode()==Null)number.chop(1);
    const auto white=[](QChar c){return c.unicode()==Space||(c.unicode()>=ParseWhiteFirst&&c.unicode()<=ParseWhiteLast);};
    while(!number.isEmpty()&&white(number.front()))number.remove(0,1);
    while(!number.isEmpty()&&white(number.back()))number.chop(1);
    const bool negative=!number.isEmpty()&&number.front().unicode()==Minus;
    if(!number.isEmpty()&&(number.front().unicode()==Plus||negative))number.remove(0,1);
    if(number.isEmpty())return {};
    std::uint32_t parsed=0;
    for(auto c:number){if(c.unicode()<DigitFirst||c.unicode()>DigitLast)return {};parsed=parsed*DecimalBase+c.unicode()-DigitFirst;}
    if(parsed>=MaximumVolume||(negative&&parsed!=0))return {};
    return std::uint8_t(parsed);
}
std::string volumeName(std::uint8_t number){auto suffix=std::to_string(number);suffix.insert(suffix.begin(),VolumeDigits-suffix.size(),DigitFirst);return std::string(VolumePrefix)+suffix;}
std::string decodeName(Bytes raw){
    std::string result;result.reserve(raw.size());for(auto c:raw)result.push_back(char(c&ValueMask));
    while(!result.empty()&&(result.back()==Space||result.back()==Null))result.pop_back();return result;
}
std::vector<std::uint8_t> encodeBinary(Bytes content,std::uint16_t address){
    if(content.size()>MaximumBinarySize)throw MediaError(DiagnosticCode::AppleDosBinarySize);
    std::vector<std::uint8_t> output(BinaryHeader+content.size());Binary::writeLittle<std::uint16_t>(output,BinaryAddress,address);Binary::writeLittle<std::uint16_t>(output,BinaryLength,std::uint16_t(content.size()));std::copy(content.begin(),content.end(),output.begin()+BinaryHeader);return output;
}
std::optional<BinaryContent> decodeBinary(Bytes stored){
    if(stored.size()<BinaryHeader)return {};
    const auto length=Binary::little<std::uint16_t>(stored,BinaryLength);if(length>stored.size()-BinaryHeader)return {};
    const auto payload=stored.subspan(BinaryHeader,length);return BinaryContent{{payload.begin(),payload.end()},Binary::little<std::uint16_t>(stored,BinaryAddress)};
}
}
