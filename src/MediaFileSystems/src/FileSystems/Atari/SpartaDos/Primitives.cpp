#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::SpartaDos::Internal {
using namespace Constants;
bool has(std::uint8_t value,Flags flag)noexcept{return (value&std::uint8_t(flag))==std::uint8_t(flag);}
std::string name(Bytes bytes){std::string value;value.reserve(bytes.size());for(auto c:bytes)value.push_back(char(c<=AsciiMaximum?c:AsciiReplacement));while(!value.empty()&&(value.back()==NamePadding||value.back()==Null))value.pop_back();return value;}
std::string fileName(Bytes bytes,Bytes extension){auto value=name(bytes);const auto ext=name(extension);if(!ext.empty()){value+=ExtensionSeparator;value+=ext;}return value;}
std::uint32_t length(Bytes bytes,std::size_t offset){if(offset>bytes.size()||bytes.size()-offset<PackedLengthSize)throw MediaError(DiagnosticCode::ReadOutOfBounds);return Binary::little<std::uint16_t>(bytes,offset)|(std::uint32_t(bytes[offset+PackedLengthSize-1])<<LengthHighShift);}
std::pair<bool,std::optional<Timestamp>> timestamp(Bytes date,Bytes time){if(std::all_of(date.begin(),date.end(),[](auto x){return x==0;})&&std::all_of(time.begin(),time.end(),[](auto x){return x==0;}))return {true,{}};Timestamp value;value.year=YearBase+date[YearIndex];value.month=date[MonthIndex];value.day=date[DayIndex];value.hour=time[HourIndex];value.minute=time[MinuteIndex];value.second=time[SecondIndex];return validTimestamp(value)?std::pair<bool,std::optional<Timestamp>>{true,value}:std::pair<bool,std::optional<Timestamp>>{false,{}};}
}
