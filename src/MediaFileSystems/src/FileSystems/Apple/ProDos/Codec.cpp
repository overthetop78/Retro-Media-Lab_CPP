#include "Internal.h"
#include <RetroMediaLab/MediaFileSystems/AppleRecognition.h>
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::ProDos {
using namespace Constants;
bool validName(std::string_view name) noexcept{
    if(name.empty()||name.size()>MaximumNameLength||name.front()<UpperFirst||name.front()>UpperLast)return false;
    return std::all_of(name.begin()+1,name.end(),[](auto c){return (c>=UpperFirst&&c<=UpperLast)||(c>=DigitFirst&&c<=DigitLast)||c==Dot;});
}
std::string readName(Bytes data,std::int64_t offset){
    if(offset<0||std::uint64_t(offset)>=data.size())throw MediaError(DiagnosticCode::ReadOutOfBounds);
    const auto length=data[std::size_t(offset)]&NameLengthMask;
    if(std::uint64_t(offset)+NameOffset+length>data.size())throw MediaError(DiagnosticCode::ReadOutOfBounds);
    std::string name;name.reserve(length);for(auto c:data.subspan(std::size_t(offset)+NameOffset,length))name.push_back(c<=std::numeric_limits<std::int8_t>::max()?char(c):char(AsciiReplacement));return name;
}
std::optional<Timestamp> readTime(Bytes data,std::int64_t offset){
    if(offset<0)throw MediaError(DiagnosticCode::ReadOutOfBounds);
    if(std::uint64_t(offset)>data.size()||DateTimeLength>data.size()-std::size_t(offset))return {};
    const auto date=Binary::little<std::uint16_t>(data,std::size_t(offset)),time=Binary::little<std::uint16_t>(data,std::size_t(offset)+sizeof(std::uint16_t));
    auto year=BaseYear+(date>>YearShift);if(year<PivotYear)year+=YearCentury;
    Timestamp result{year,(date>>MonthShift)&MonthMask,date&DayMask,time>>HourShift,time&MinuteMask};
    return validTimestamp(result)?std::optional(result):std::nullopt;
}
void writeTime(MutableBytes data,std::int64_t offset,const std::optional<Timestamp>& value){
    if(offset<0||std::uint64_t(offset)>data.size()||DateTimeLength>data.size()-std::size_t(offset))throw MediaError(DiagnosticCode::ReadOutOfBounds);
    if(!value){std::fill_n(data.begin()+offset,DateTimeLength,std::uint8_t(0));return;}
    if(!validTimestamp(*value))throw MediaError(DiagnosticCode::ProDosTimestamp);
    const auto date=(value->year%YearCentury)<<YearShift|value->month<<MonthShift|value->day;
    const auto time=value->hour<<HourShift|value->minute;
    Binary::writeLittle(data,std::size_t(offset),std::uint16_t(date));Binary::writeLittle(data,std::size_t(offset)+sizeof(std::uint16_t),std::uint16_t(time));
}
namespace Internal {
bool sameId(std::string_view a,std::string_view b) noexcept{if(a.size()!=b.size())return false;const auto lower=[](char c){return c>=UpperFirst&&c<=UpperLast?char(c+CaseDelta):c;};for(std::size_t i=0;i<a.size();++i)if(lower(a[i])!=lower(b[i]))return false;return true;}
std::string hexByte(std::uint8_t value){std::string text(HexByteDigits,HexDigits[0]);text[0]=HexDigits[value>>HexShift];text[1]=HexDigits[value&HexMask];return text;}
std::string storageName(Storage storage){switch(storage){case Storage::Seedling:return std::string(SeedlingName);case Storage::Sapling:return std::string(SaplingName);case Storage::Tree:return std::string(TreeName);default:return std::to_string(std::uint8_t(storage));}}
std::uint32_t read24(Bytes data,std::size_t offset){if(offset>data.size()||data.size()-offset<sizeof(std::uint16_t)+1)throw MediaError(DiagnosticCode::ReadOutOfBounds);return Binary::little<std::uint16_t>(data,offset)|std::uint32_t(data[offset+sizeof(std::uint16_t)])<<(BitsPerByte*sizeof(std::uint16_t));}
void write16(MutableBytes data,std::size_t offset,std::int64_t value){if(value<0||value>MaximumWord)throw MediaError(DiagnosticCode::ProDosGeometry,{},ErrorKind::ArgumentOutOfRange);Binary::writeLittle(data,offset,std::uint16_t(value));}
void write24(MutableBytes data,std::size_t offset,std::int64_t value){if(value<0||value>MaximumFileLength)throw MediaError(DiagnosticCode::ProDosGeometry,{},ErrorKind::ArgumentOutOfRange);if(offset>data.size()||data.size()-offset<sizeof(std::uint16_t)+1)throw MediaError(DiagnosticCode::ReadOutOfBounds);write16(data,offset,value&MaximumWord);data[offset+sizeof(std::uint16_t)]=std::uint8_t(value>>(BitsPerByte*sizeof(std::uint16_t)));}
void writeIndex(MutableBytes data,std::size_t index,std::int32_t value){if(index>=IndexPointerCount||data.size()<BlockSize)throw MediaError(DiagnosticCode::ReadOutOfBounds);data[index]=std::uint8_t(value);data[index+IndexHighBytesOffset]=std::uint8_t(value>>BitsPerByte);}
std::int32_t readIndex(Bytes data,std::size_t index){if(index>=IndexPointerCount||data.size()<BlockSize)throw MediaError(DiagnosticCode::ReadOutOfBounds);return data[index]|std::int32_t(data[index+IndexHighBytesOffset])<<BitsPerByte;}
std::optional<Header> header(const SectorImage& image,std::stop_token token){
    checkCancellation(token);if(image.geometry().blockSize!=BlockSize)return {};const auto root=image.block(RootBlock,token);if(!root||!Apple::validProDosHeader(root->data))return {};
    return Header{readName(root->data,HeaderOffset),Binary::little<std::uint16_t>(root->data,BitmapBlockOffset),Binary::little<std::uint16_t>(root->data,TotalBlocksOffset),readTime(root->data,HeaderOffset+CreatedDateOffset),root->data};
}
void setType(FileSystemEntry& entry,std::uint8_t type){
    const auto found=std::find(Types.begin(),Types.end(),type);const auto index=std::size_t(found-Types.begin());
    entry.commentTranslationDomain=Domain;entry.commentResourceKey=TypeKeys[index];
    if(found!=Types.end())entry.comment=TypeNames[index];else{entry.comment=std::string(UnknownTypePrefix)+hexByte(type);entry.commentArguments={hexByte(type)};}
}
}
}
