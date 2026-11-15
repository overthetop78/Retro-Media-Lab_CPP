#include "Primitives.h"
#include "Constants.h"
#include "MacRomanTable.h"
#include "../../../Primitives/Identifiers.h"
#include <QString>
#include <chrono>
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::Macintosh {
std::string fileType(Bytes data){
    std::string result;for(auto c:data)result.push_back(char(c<=Constants::AsciiMaximum?c:Constants::AsciiReplacement));
    while(!result.empty()&&(result.back()==Constants::Null||result.back()==Constants::Space))result.pop_back();
    const auto first=result.find_first_not_of(std::string{Constants::Null,Constants::Space});if(first==std::string::npos)return {};result=result.substr(first);
    if(std::all_of(result.begin(),result.end(),[](unsigned char value){return value==Constants::Space||(value>=Constants::FirstAsciiWhitespace&&value<=Constants::LastAsciiWhitespace);}))return {};
    return result;
}
namespace {
std::uint16_t ordinalUpper(QChar c){const auto upper=c.toUpper().unicode();return c.unicode()>Constants::AsciiMaximum&&upper<=Constants::AsciiMaximum?c.unicode():upper;}
}
bool lessName(const std::string& a,const std::string& b){
    const auto left=Identifiers::utf8(a),right=Identifiers::utf8(b);const auto count=std::min(left.size(),right.size());
    for(qsizetype index=0;index<count;++index){const auto x=ordinalUpper(left[index]),y=ordinalUpper(right[index]);if(x!=y)return x<y;}
    return left.size()<right.size();
}
std::string decodeName(Bytes data){
    std::u16string decoded;decoded.reserve(data.size());for(auto value:data){const auto c=Constants::MacRoman[value];decoded.push_back(c==Constants::NativeSeparator?Constants::PathSeparator:c);}
    return QString::fromUtf16(decoded.data(),qsizetype(decoded.size())).toUtf8().toStdString();
}
std::string pascalString(Bytes data,std::size_t offset,std::size_t maximum){
    if(offset>=data.size())throw MediaError(DiagnosticCode::ReadOutOfBounds);
    const auto count=std::min(std::size_t(data[offset]),maximum);
    if(count>data.size()-offset-1)throw MediaError(DiagnosticCode::ReadOutOfBounds);
    return decodeName(data.subspan(offset+1,count));
}
std::int32_t storageReference(std::uint32_t value) noexcept{return std::int32_t(std::min(value,Constants::MaximumReference));}
std::optional<Timestamp> fromSeconds(std::int64_t value){
    if(value==0)return {};
    using namespace std::chrono;using namespace Constants;
    const sys_days epoch=year(EpochYear)/month(EpochMonth)/day(EpochDay);
    const auto lower=duration_cast<seconds>(sys_days(year(MinimumYear)/month(1)/day(1))-epoch).count();
    const auto upper=duration_cast<seconds>(sys_days(year(MaximumYear+1)/month(1)/day(1))-epoch).count();
    if(value<lower||value>=upper)return {};
    const auto instant=sys_seconds(epoch)+seconds(value);const auto date=floor<days>(instant);const year_month_day ymd(date);const hh_mm_ss time(instant-date);
    Timestamp result;result.year=int(ymd.year());result.month=int(unsigned(ymd.month()));result.day=int(unsigned(ymd.day()));result.hour=int(time.hours().count());result.minute=int(time.minutes().count());result.second=int(time.seconds().count());return result;
}
}
