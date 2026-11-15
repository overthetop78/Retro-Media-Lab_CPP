#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Ucsd::Internal {
using namespace Constants;
std::uint16_t word(Bytes bytes,std::size_t offset,ByteOrder order){return order==ByteOrder::LittleEndian?Binary::little<std::uint16_t>(bytes,offset):Binary::big<std::uint16_t>(bytes,offset);}
bool validName(Bytes value)noexcept{if(value.empty())return false;return std::all_of(value.begin(),value.end(),[](auto c){return c>=MinimumNameCharacter&&c<MaximumNameCharacterExclusive;});}
std::string name(Bytes field,std::int32_t maximum){if(field.empty())return {};const auto length=field[0];if(length==0||length>maximum||length>=field.size()||!validName(field.subspan(1,length)))return {};return std::string(reinterpret_cast<const char*>(field.data()+1),length);}
std::optional<Timestamp> date(std::uint16_t value){if(value==0)return {};const auto shortYear=(value>>YearShift)&YearMask;Timestamp result;result.year=(shortYear>=CenturyPivot?NineteenthCenturyBase:TwentiethCenturyBase)+shortYear;result.month=(value>>MonthShift)&MonthMask;result.day=value&DayMask;return validTimestamp(result)?std::optional<Timestamp>(result):std::nullopt;}
bool directoryEnd(std::int32_t value)noexcept{return value==ShortDirectoryEnd||value==LongDirectoryEnd;}
std::optional<ByteOrder> detectOrder(Bytes bytes)noexcept{if(bytes.size()<DirectoryEndOffset+sizeof(std::uint16_t))return {};if(directoryEnd(bytes[DirectoryEndOffset])&&bytes[DirectoryEndOffset+1]==0)return ByteOrder::LittleEndian;if(directoryEnd(bytes[DirectoryEndOffset+1])&&bytes[DirectoryEndOffset]==0)return ByteOrder::BigEndian;return {};}
std::string join(const std::vector<std::int32_t>& blocks){std::string result;for(auto block:blocks){if(!result.empty())result+=Separator;result+=std::to_string(block);}return result;}
bool sameFormat(std::string_view value)noexcept{if(value.size()!=Format.size())return false;for(std::size_t i=0;i<value.size();++i){const auto c=std::uint8_t(value[i]);if((c>=UpperA&&c<=UpperZ?char(c+LowerDelta):char(c))!=Format[i])return false;}return true;}
std::size_t kindIndex(FileKind kind)noexcept{const auto value=std::int64_t(kind);return value>=0&&std::uint64_t(value)<TypeNames.size()?std::size_t(value):0;}
}
namespace RetroMediaLab::MediaFileSystems::Ucsd {
std::string_view kindName(FileKind kind)noexcept{return Constants::TypeNames[Internal::kindIndex(kind)];}
}
