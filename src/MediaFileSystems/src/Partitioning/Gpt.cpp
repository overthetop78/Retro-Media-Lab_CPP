#include <RetroMediaLab/MediaFileSystems/Volumes.h>
#include "Helpers.h"
#include <limits>
#include <vector>
namespace RetroMediaLab::MediaFileSystems {
namespace {
using namespace PartitionConstants;
using namespace BinaryConstants;
struct Header {
    std::uint32_t revision;
    std::uint64_t current, backup, firstUsable, lastUsable;
    std::string diskId;
    std::uint64_t arrayLba;
    std::uint32_t count, size, arrayCrc;
};
std::vector<std::uint8_t> block(const BlockMedia& media,std::uint64_t lba,std::stop_token token) {
    if(lba>=media.blockCount())throw MediaError(DiagnosticCode::GptStructureOutside);
    std::vector<std::uint8_t> bytes(media.blockSize());media.readExactly(lba*media.blockSize(),bytes,token);return bytes;
}
Header parse(Bytes bytes,const BlockMedia& media,std::uint64_t expected) {
    const auto location=std::vector<std::string>{std::to_string(expected)};
    if(bytes.size()<MinimumGptHeaderSize || !Binary::hasGptSignature(bytes))throw MediaError(DiagnosticCode::GptHeaderSignature,location);
    const auto revision=Binary::little<std::uint32_t>(bytes,GptRevisionOffset),size=Binary::little<std::uint32_t>(bytes,GptHeaderSizeOffset);
    if(revision<MinimumGptRevision || size<MinimumGptHeaderSize || size>bytes.size())throw MediaError(DiagnosticCode::GptHeaderDimensions,location);
    const auto expectedCrc=Binary::little<std::uint32_t>(bytes,GptHeaderCrcOffset);
    std::vector<std::uint8_t> checked(bytes.begin(),bytes.begin()+size);
    std::fill_n(checked.begin()+GptHeaderCrcOffset,CrcFieldSize,0);
    if(Binary::crc32(checked)!=expectedCrc)throw MediaError(DiagnosticCode::GptHeaderCrc,location);
    Header header{revision,Binary::little<std::uint64_t>(bytes,GptCurrentLbaOffset),Binary::little<std::uint64_t>(bytes,GptBackupLbaOffset),
        Binary::little<std::uint64_t>(bytes,GptFirstUsableOffset),Binary::little<std::uint64_t>(bytes,GptLastUsableOffset),Binary::guid(bytes.subspan(GptDiskIdOffset,GuidLength)),
        Binary::little<std::uint64_t>(bytes,GptArrayLbaOffset),Binary::little<std::uint32_t>(bytes,GptEntryCountOffset),Binary::little<std::uint32_t>(bytes,GptEntrySizeOffset),Binary::little<std::uint32_t>(bytes,GptArrayCrcOffset)};
    if(header.current!=expected || header.backup>=media.blockCount() || header.firstUsable>header.lastUsable || header.lastUsable>=media.blockCount()
        || header.arrayLba>=media.blockCount() || header.count==0 || header.size<MinimumGptEntrySize || header.size%GptEntryAlignment!=0)
        throw MediaError(DiagnosticCode::GptHeaderBounds,location);
    const auto length=std::uint64_t(header.count)*header.size;
    // Check subtraction before multiplication; corrupt arrays must never wrap the capacity.
    if(length>MaximumGptArraySize || length>std::uint64_t(std::numeric_limits<std::int32_t>::max()) || length>media.capacity()
        || header.arrayLba>(media.capacity()-length)/media.blockSize())
        throw MediaError(DiagnosticCode::GptArrayOutside,{std::to_string(header.arrayLba)});
    return header;
}
std::vector<std::uint8_t> entryArray(const BlockMedia& media,const Header& header,std::stop_token token) {
    std::vector<std::uint8_t> data(std::size_t(header.count)*header.size);media.readExactly(header.arrayLba*media.blockSize(),data,token);return data;
}
void secondary(const BlockMedia& media,const Header& primary,VolumeDetectionResult& result,std::stop_token token) {
    try {
        const auto bytes=block(media,primary.backup,token);const auto backup=parse(bytes,media,primary.backup);
        if(backup.backup!=primary.current || backup.firstUsable!=primary.firstUsable || backup.lastUsable!=primary.lastUsable || backup.diskId!=primary.diskId
            || backup.count!=primary.count || backup.size!=primary.size || backup.arrayCrc!=primary.arrayCrc)
            result.diagnostics.push_back({DiagnosticCode::GptSecondaryMismatch,{}});
        const auto entries=entryArray(media,backup,token);
        if(Binary::crc32(entries)!=backup.arrayCrc)result.diagnostics.push_back({DiagnosticCode::GptSecondaryArrayCrc,{}});
    } catch(const MediaError& error) {
        if(error.kind()!=ErrorKind::InvalidData)throw;
        result.diagnostics.push_back(error.diagnostic());
    }
}
std::optional<std::string> partitionName(Bytes entry) {
    const auto length=std::min(std::size_t(GptNameMaximumBytes),entry.size()-GptNameOffset);
    std::u16string name;
    for(std::size_t offset=0;offset+sizeof(char16_t)<=length;offset+=sizeof(char16_t))name.push_back(char16_t(Binary::little<std::uint16_t>(entry,GptNameOffset+offset)));
    while(!name.empty() && name.back()==0)name.pop_back();
    const auto value=QString::fromUtf16(name.data(),qsizetype(name.size()));
    return value.trimmed().isEmpty()?std::nullopt:std::optional<std::string>(value.toUtf8().toStdString());
}
void partitions(const BlockMedia& media,const Header& header,Bytes entries,VolumeDetectionResult& result,std::stop_token token) {
    for(std::uint32_t i=0;i<header.count;++i) {
        checkCancellation(token);
        const auto entry=entries.subspan(std::size_t(i)*header.size,header.size);
        if(Binary::zero(entry.first(GuidLength)))continue;
        const auto first=Binary::little<std::uint64_t>(entry,GptEntryFirstOffset),last=Binary::little<std::uint64_t>(entry,GptEntryLastOffset);
        const auto number=i+1;
        if(Binary::zero(entry.subspan(GptEntryIdOffset,GuidLength)) || first>last || first<header.firstUsable || last>header.lastUsable || last>=media.blockCount()) {
            result.diagnostics.push_back({DiagnosticCode::GptPartitionBounds,{std::to_string(number)}});continue;
        }
        result.volumes.emplace_back(Binary::volumeCoordinate(first*media.blockSize()), Binary::volumeCoordinate((last-first+1)*media.blockSize()),
            std::string(VolumeConstants::Partition), std::string(VolumeConstants::Gpt), std::int32_t(number), std::nullopt, std::nullopt, std::nullopt,
            Binary::guid(entry.first(GuidLength)), Binary::guid(entry.subspan(GptEntryIdOffset,GuidLength)), partitionName(entry));
    }
}
}
std::optional<VolumeDetectionResult> detectGpt(const BlockMedia& media,std::stop_token token) {
    checkCancellation(token);
    if(!media.isHardDisk() || media.blockCount()<MinimumGptBlockCount)return std::nullopt;
    const auto bytes=block(media,1,token);if(!Binary::hasGptSignature(bytes))return std::nullopt;
    VolumeDetectionResult result;std::optional<Header> primary;
    try { primary=parse(bytes,media,1); }
    catch(const MediaError& error) {if(error.kind()!=ErrorKind::InvalidData)throw;result.diagnostics.push_back(error.diagnostic());return result;}
    const auto entries=entryArray(media,*primary,token);
    if(Binary::crc32(entries)!=primary->arrayCrc)result.diagnostics.push_back({DiagnosticCode::GptPrimaryArrayCrc,{}});
    secondary(media,*primary,result,token);partitions(media,*primary,entries,result,token);Binary::overlaps(result,DiagnosticCode::GptOverlap);return result;
}
}
