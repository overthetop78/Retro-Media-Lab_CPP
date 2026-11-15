#include "Internal.h"
#include "../../../Primitives/MetadataConstants.h"
#include "../../../Primitives/Identifiers.h"
#include <QString>
#include <chrono>
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::AmigaDos {
using namespace Constants;
bool isFast(Variant variant) noexcept{return (std::uint8_t(variant)&1)!=0;}
bool supportsLongNames(Variant variant) noexcept{return variant>=Variant::OfsLongNames;}
std::string_view fileSystemId(Variant variant) noexcept{const auto value=std::uint8_t(variant);return value<FileSystemIds.size()?FileSystemIds[value]:FileSystemId;}
EntryKind entryKind(std::int32_t type) noexcept{switch(type){case DirectorySecondaryType:return EntryKind::Directory;case FileSecondaryType:return EntryKind::File;case FileLinkSecondaryType:case DirectoryLinkSecondaryType:case HardLinkSecondaryType:return EntryKind::Link;default:return EntryKind::Unknown;}}
bool validChecksum(Bytes data){if(data.size()!=BlockSize)return false;std::uint32_t sum=0;for(std::size_t offset=0;offset<data.size();offset+=WordSize)sum+=Internal::unsignedInteger(data,offset);return sum==0;}
bool unavailableBlock(Bytes data){if(data.empty()||data.size()%BadSectorPattern.size()!=0)return false;for(std::size_t i=0;i<data.size();++i)if(data[i]!=std::uint8_t(BadSectorPattern[i%BadSectorPattern.size()]))return false;return true;}
std::string readName(Bytes data,std::int64_t offset,std::int64_t maximum){
    if(offset<0||maximum<0||std::uint64_t(offset)>=data.size())return {};
    auto length=std::min(std::uint64_t(data[std::size_t(offset)]),std::min(std::uint64_t(maximum),std::uint64_t(data.size())-std::uint64_t(offset)-1));
    while(length>0&&data[std::size_t(offset)+length]==0)--length;
    return QString::fromLatin1(reinterpret_cast<const char*>(data.data()+offset+1),qsizetype(length)).toUtf8().toStdString();
}
std::string readEntryName(Bytes data,Variant variant){auto name=readName(data,OrdinaryNameOffset,OrdinaryNameMaximumLength);if(name.empty()&&supportsLongNames(variant))name=readName(data,LongNameOffset,LongNameMaximumLength);return name;}
bool validName(std::string_view name){
    const auto text=QString::fromUtf8(name.data(),qsizetype(name.size()));
    if(text.isEmpty()||text.size()>OrdinaryNameMaximumLength||text.toUtf8().toStdString()!=name)return false;
    for(const auto c:text)if(c.unicode()>Latin1Maximum||c==PathSeparator||c==VolumeSeparator)return false;return true;
}
std::optional<Timestamp> readTime(Bytes data,std::int64_t offset){
    if(offset<0||std::uint64_t(offset)>data.size()||WordSize*DosVariantOffset>data.size()-std::uint64_t(offset))return {};
    const auto dayCount=Internal::integer(data,std::size_t(offset)),minute=Internal::integer(data,std::size_t(offset)+WordSize),tick=Internal::integer(data,std::size_t(offset)+WordSize*BootBlockCount);
    if(dayCount<0||minute<0||minute>=MinutesPerDay||tick<0||tick>=SecondsPerMinute*TicksPerSecond)return {};
    using namespace std::chrono;const auto epoch=sys_days(year(EpochYear)/1/1);
    // The public model follows DateTimeOffset's supported calendar, not chrono's wider year range.
    const auto maximum=sys_days(year(MetadataConstants::MaximumYear)/month(unsigned(MetadataConstants::MonthsPerYear))/day(unsigned(MetadataConstants::MaximumMonthDays)));
    if(days(dayCount)>maximum-epoch)return {};
    const auto date=year_month_day(epoch+days(dayCount));
    Timestamp result;result.year=int(date.year());result.month=int(unsigned(date.month()));result.day=int(unsigned(date.day()));result.hour=minute/MinutesPerHour;result.minute=minute%MinutesPerHour;result.second=tick/TicksPerSecond;result.nanosecond=(tick%TicksPerSecond)*NanosecondsPerTick;return result;
}
namespace Internal {
bool recordedIntegrity(const SectorBlock& block) noexcept{return block.integrityValid!=false&&block.headerIntegrityValid!=false&&block.dataIntegrityValid!=false;}
bool dosPrefix(Bytes data){return data.size()>DosVariantOffset&&std::equal(DosSignature.begin(),DosSignature.end(),data.begin());}
void sort(std::vector<FileSystemEntry>& entries){
    const auto folded=[](const std::string& text){auto value=QString::fromUtf8(text);for(auto& c:value)c=c.toUpper();return value.toStdU16String();};
    std::stable_sort(entries.begin(),entries.end(),[&](const auto&a,const auto&b){if((a.kind==EntryKind::Directory)!=(b.kind==EntryKind::Directory))return a.kind==EntryKind::Directory;return folded(a.name)<folded(b.name);});
}
bool recoverableName(std::string_view name){const auto text=Identifiers::utf8(name);if(text.trimmed().isEmpty())return false;for(const auto c:text)if(c.unicode()<=ControlFirstEnd||(c.unicode()>=ControlSecondStart&&c.unicode()<=ControlSecondEnd))return false;return true;}
void setChecksum(MutableBytes data,std::size_t offset){Binary::writeBig(data,offset,std::uint32_t(0));std::uint32_t sum=0;for(std::size_t index=0;index<data.size();index+=WordSize)sum+=unsignedInteger(data,index);Binary::writeBig(data,offset,std::uint32_t(0)-sum);}
std::uint32_t nameHash(std::string_view name){const auto text=QString::fromUtf8(name.data(),qsizetype(name.size()));auto value=std::uint32_t(text.size());for(const auto byte:text.toLatin1())value=(value*HashMultiplier+std::uint8_t(QChar::toUpper(uchar(byte))))&HashMask;return value%RootHashTableEntryCount;}
void writeDate(MutableBytes data,std::size_t offset,const Timestamp& value){
    if(!validTimestamp(value))throw MediaError(DiagnosticCode::AmigaInvalidEntry,{std::string(RootPath)});
    using namespace std::chrono;const auto epoch=sys_days(year(EpochYear)/1/1);const auto date=sys_days(year(value.year)/month(unsigned(value.month))/day(unsigned(value.day)));
    auto elapsed=duration_cast<seconds>(date-epoch)+hours(value.hour)+minutes(value.minute-value.offsetMinutes)+seconds(value.second);
    auto fraction=value.nanosecond;if(elapsed<seconds::zero()){elapsed=seconds::zero();fraction=0;}
    const auto count=floor<days>(elapsed);const auto within=elapsed-count;const auto minute=floor<minutes>(within);
    const auto ticks=duration_cast<seconds>(within-minute).count()*TicksPerSecond+fraction/NanosecondsPerTick;
    Binary::writeBig(data,offset,std::int32_t(count.count()));Binary::writeBig(data,offset+WordSize,std::int32_t(minute.count()));Binary::writeBig(data,offset+WordSize*BootBlockCount,std::int32_t(ticks));
}
}
}
