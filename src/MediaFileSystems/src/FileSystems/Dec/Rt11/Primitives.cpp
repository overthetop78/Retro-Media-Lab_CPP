#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Rt11 {
using namespace Constants;
std::string radix50(std::uint16_t word){std::string result(CharacterCount,Space);result[0]=Alphabet[word/FirstDivisor%Base];result[1]=Alphabet[word/SecondDivisor%Base];result[CharacterCount-1]=Alphabet[word%Base];return result;}
std::optional<Timestamp> decodeDate(std::uint16_t word){if(word==0)return {};Timestamp value;value.day=word&DayMask;value.month=(word>>MonthShift)&MonthMask;value.year=BaseYear+((word>>YearShift)&YearMask)+((word>>AgeShift)&AgeMask)*YearsPerAge;return validTimestamp(value)?std::optional<Timestamp>(value):std::nullopt;}
std::string decodeAscii(Bytes source){std::string value;value.reserve(source.size());for(auto c:source)value.push_back(char(c<=AsciiMaximum?c:AsciiReplacement));while(!value.empty()&&(value.back()==Null||value.back()==Space))value.pop_back();return value;}
bool looksLike(Bytes block){if(block.size()!=BlockSize)return false;const auto directory=Binary::little<std::uint16_t>(block,DirectoryBlockOffset);const auto signature=decodeAscii(block.subspan(SystemIdOffset,SystemIdLength));return directory>=MinimumDirectoryBlock&&directory<MaximumDirectoryBlockExclusive&&signature.starts_with(SystemSignature);}
}
namespace RetroMediaLab::MediaFileSystems::Rt11::Internal {
using namespace Constants;
bool has(std::uint16_t value,EntryStatus flag)noexcept{return (value&std::uint16_t(flag))==std::uint16_t(flag);}
bool sameFormat(std::string_view value)noexcept{if(value.size()!=Format.size())return false;for(std::size_t i=0;i<value.size();++i){const auto c=std::uint8_t(value[i]);if((c>=UpperA&&c<=UpperZ?char(c+LowerDelta):char(c))!=Format[i])return false;}return true;}
std::string join(const std::vector<std::int32_t>& blocks){std::string value;for(auto n:blocks){if(!value.empty())value+=Separator;value+=std::to_string(n);}return value;}
}
