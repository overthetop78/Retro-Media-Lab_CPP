#include <RetroMediaLab/MediaFileSystems/FileSystemModels.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include "MetadataConstants.h"
#include <chrono>
namespace RetroMediaLab::MediaFileSystems {
bool validTimestamp(const Timestamp& value) noexcept {
    using namespace MetadataConstants;
    if(value.month<1 || value.month>MonthsPerYear || value.day<1 || value.day>MaximumMonthDays || value.year<MinimumYear || value.year>MaximumYear || value.hour<0 || value.hour>=HoursPerDay || value.minute<0 || value.minute>=MinutesPerHour
        || value.second<0 || value.second>=SecondsPerMinute || value.nanosecond<0 || value.nanosecond>=NanosecondsPerSecond
        || value.offsetMinutes < -MaximumOffsetMinutes || value.offsetMinutes>MaximumOffsetMinutes)return false;
    return std::chrono::year_month_day(std::chrono::year(value.year),std::chrono::month(unsigned(value.month)),std::chrono::day(unsigned(value.day))).ok();
}
std::shared_ptr<const SectorBlock> requireSectorBlock(const SectorImage& image,std::uint64_t logical,std::stop_token token) {
    const auto block=image.block(logical,token);
    if(!block)throw MediaError(DiagnosticCode::SectorMissingBlock,{std::to_string(logical)});
    if(!image.allowsVariableBlockSize() && block->data.size()!=image.geometry().blockSize)
        throw MediaError(DiagnosticCode::SectorInvalidBlockSize,{std::to_string(logical),std::to_string(block->data.size()),std::to_string(image.geometry().blockSize)});
    return block;
}
}
