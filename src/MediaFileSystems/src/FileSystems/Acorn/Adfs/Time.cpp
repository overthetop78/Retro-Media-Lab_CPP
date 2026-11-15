#include <RetroMediaLab/MediaFileSystems/Adfs.h>
#include "Constants.h"
#include <chrono>
#include <iomanip>
#include <sstream>
namespace RetroMediaLab::MediaFileSystems::Adfs {
using namespace Constants;
bool hasTimestamp(std::uint32_t load) noexcept{return (load&TimestampMask)==TimestampMask;}
std::optional<Timestamp> decodeTime(std::uint32_t load,std::uint32_t execute){
    if(!hasTimestamp(load))return {};
    using namespace std::chrono;const auto ticks=(std::uint64_t(load&ByteMask)<<HighTimeBits)|execute;
    const auto instant=sys_days(year(EpochYear)/1/1)+seconds(ticks/CentisecondsPerSecond);
    const auto day=floor<days>(instant);const year_month_day date(day);const hh_mm_ss clock(instant-day);
    Timestamp result;result.year=int(date.year());result.month=int(unsigned(date.month()));result.day=int(unsigned(date.day()));
    result.hour=int(clock.hours().count());result.minute=int(clock.minutes().count());result.second=int(clock.seconds().count());
    result.nanosecond=int(ticks%CentisecondsPerSecond)*NanosecondsPerCentisecond;return result;
}
std::string describe(std::uint32_t load,std::uint32_t execute){
    std::ostringstream text;text.imbue(std::locale::classic());text<<std::uppercase<<std::hex<<std::setfill(HexPadding);
    if(hasTimestamp(load))text<<RiscDescription<<std::setw(TypeHexWidth)<<((load>>BitsPerByte)&TypeMask)<<LoadDescription;
    else text<<AdfsDescription;
    text<<std::setw(AddressHexWidth)<<load<<ExecuteDescription<<std::setw(AddressHexWidth)<<execute;return text.str();
}
}
