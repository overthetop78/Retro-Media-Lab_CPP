#include <RetroMediaLab/MediaFileSystems/Fat.h>
#include "Constants.h"
#include "../../Primitives/Binary.h"
#include <QString>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Fat {
using namespace FatConstants;
std::optional<BpbGeometry> detectBpbGeometry(Bytes boot,std::optional<std::uint64_t> imageLength) {
    using namespace FatBootSectorLayout;
    if(boot.size()<MinimumLength)return std::nullopt;
    const auto sectorSize=Binary::little<std::uint16_t>(boot,BytesPerSectorOffset);
    std::uint32_t total=Binary::little<std::uint16_t>(boot,TotalSectors16Offset);
    if(total==0)total=Binary::little<std::uint32_t>(boot,TotalSectors32Offset);
    if(total>std::uint32_t(std::numeric_limits<std::int32_t>::max()))return std::nullopt;
    const auto sectors=Binary::little<std::uint16_t>(boot,SectorsPerTrackOffset),heads=Binary::little<std::uint16_t>(boot,HeadCountOffset);
    if(sectorSize!=SectorSize || total==0 || sectors==0 || sectors>MaximumSectorsPerTrack || heads==0 || heads>MaximumHeadCount || total%(sectors*heads)!=0)return std::nullopt;
    if(imageLength && (*imageLength%sectorSize!=0 || total!=*imageLength/sectorSize))return std::nullopt;
    const auto cylinders=total/(sectors*heads);
    if(cylinders==0 || cylinders>MaximumCylinderCount)return std::nullopt;
    return BpbGeometry{sectorSize,total,cylinders,heads,sectors};
}
std::optional<Timestamp> decodeDateTime(std::uint16_t date,std::uint16_t time) {
    using namespace FatDateTime;
    const Timestamp value{int(BaseYear+(date>>YearShift)),int(date>>MonthShift&MonthMask),int(date&DayMask),int(time>>HourShift),int(time>>MinuteShift&MinuteMask),int((time&SecondMask)*SecondMultiplier)};
    return validTimestamp(value)?std::optional<Timestamp>(value):std::nullopt;
}
std::string decodeFixed(Bytes bytes,std::size_t offset,std::size_t length) {
    if(offset>bytes.size() || length>bytes.size()-offset)throw MediaError(DiagnosticCode::ReadOutOfBounds);
    if(length==0)return {};
    auto value=QString::fromLatin1(reinterpret_cast<const char*>(bytes.data()+offset),qsizetype(length));
    while(!value.isEmpty() && (value.back()==QLatin1Char(FatBootSectorLayout::NullPadding) || value.back()==QLatin1Char(FatBootSectorLayout::SpacePadding)))value.chop(1);
    return value.toUtf8().toStdString();
}
std::string decodeName(Bytes bytes) {
    using namespace FatDirectoryLayout;
    auto name=QString::fromUtf8(decodeFixed(bytes,0,NameLength)).trimmed();
    const auto extension=QString::fromUtf8(decodeFixed(bytes,ExtensionOffset,ExtensionLength)).trimmed();
    if(!extension.isEmpty())name+=QLatin1Char(ExtensionSeparator)+extension;
    return name.toUtf8().toStdString();
}
std::optional<std::string> volumeLabel(Bytes root) {
    using namespace FatDirectoryLayout;
    for(std::size_t offset=0;offset+EntrySize<=root.size();offset+=EntrySize) {
        if(root[offset]==EndMarker || root[offset]==DeletedMarker)continue;
        const auto attributes=root[offset+AttributesOffset];
        if((attributes&LongFileName)==LongFileName)continue;
        if((attributes&VolumeLabelAttribute)!=0)return QString::fromUtf8(decodeFixed(root,offset,NameLength+ExtensionLength)).trimmed().toUtf8().toStdString();
    }
    return std::nullopt;
}
std::string bootVolumeLabel(Bytes boot) {
    using namespace FatBootSectorLayout;
    if(boot.size()<ExtendedBootMinimumLength)return {};
    const auto bytes=boot.subspan(VolumeLabelOffset,VolumeLabelLength);
    for(const auto byte:bytes)if(byte<PrintableAsciiStart || byte>PrintableAsciiEnd)return {};
    const auto label=QString::fromLatin1(reinterpret_cast<const char*>(bytes.data()),qsizetype(bytes.size())).trimmed();
    return label.compare(QString::fromUtf8(EmptyVolumeLabel),Qt::CaseInsensitive)==0?std::string{}:label.toUtf8().toStdString();
}
}
