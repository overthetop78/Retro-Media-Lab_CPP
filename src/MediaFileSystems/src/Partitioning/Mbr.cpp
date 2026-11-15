#include <RetroMediaLab/MediaFileSystems/Volumes.h>
#include "Helpers.h"
#include <array>
#include <limits>
#include <set>
namespace RetroMediaLab::MediaFileSystems {
namespace {
using namespace PartitionConstants;
using namespace BinaryConstants;
struct Entry { std::uint8_t type; std::uint32_t first, count; };
using Record = std::array<std::uint8_t, BootRecordSize>;
Record bootRecord(const BlockMedia& media, std::uint64_t lba, std::stop_token token) {
    if (lba >= media.blockCount()) throw MediaError(DiagnosticCode::MbrBootOutside);
    Record record{}; media.readExactly(lba * media.blockSize(), record, token); return record;
}
bool signature(Bytes record) { return record[BootSignatureOffset] == BootSignatureFirst && record[BootSignatureOffset+1] == BootSignatureLast; }
std::array<Entry, PartitionEntryCount> entries(Bytes record) {
    std::array<Entry, PartitionEntryCount> values{};
    for (std::size_t i = 0; i < values.size(); ++i) {
        const auto offset = PartitionTableOffset + i * PartitionEntrySize;
        values[i] = {record[offset + PartitionTypeOffset], Binary::little<std::uint32_t>(record, offset + PartitionLbaOffset), Binary::little<std::uint32_t>(record, offset + PartitionCountOffset)};
    }
    return values;
}
bool extended(std::uint8_t type) { return type == ExtendedChs || type == ExtendedLba || type == ExtendedLinux; }
void add(const BlockMedia& media, std::uint64_t first, std::uint64_t count, std::uint8_t type, std::uint32_t number, VolumeDetectionResult& result) {
    const auto signedLimit = std::uint64_t(std::numeric_limits<std::int64_t>::max());
    if (first == 0 || count == 0 || first > signedLimit / media.blockSize() || count > signedLimit / media.blockSize()) {
        result.diagnostics.push_back({DiagnosticCode::MbrInvalidBounds,{std::to_string(number)}}); return;
    }
    const auto start = first * media.blockSize(), length = count * media.blockSize();
    if (start > media.capacity() || length > media.capacity() - start) {
        result.diagnostics.push_back({DiagnosticCode::MbrExceedsCapacity,{std::to_string(number)}}); return;
    }
    result.volumes.emplace_back(Binary::volumeCoordinate(start), Binary::volumeCoordinate(length), std::string(VolumeConstants::Partition),
        std::string(VolumeConstants::Mbr), std::int32_t(number), std::nullopt, std::nullopt, std::nullopt, Binary::partitionType(type));
}
void chain(const BlockMedia& media, Entry container, std::uint32_t& number, VolumeDetectionResult& result, std::stop_token token) {
    const auto base = std::uint64_t(container.first), end = base + container.count;
    auto current = base; std::set<std::uint64_t> visited;
    for (std::size_t i = 0; current != 0 && i < MaximumEbrCount; ++i) {
        checkCancellation(token);
        if (!visited.insert(current).second) {result.diagnostics.push_back({DiagnosticCode::EbrLoop,{}});return;}
        if (current < base || current >= end) {result.diagnostics.push_back({DiagnosticCode::EbrOutside,{}});return;}
        Record record;
        try { record = bootRecord(media,current,token); }
        catch(const MediaError& error) { if(error.kind()!=ErrorKind::InvalidData)throw;result.diagnostics.push_back({DiagnosticCode::EbrUnreadable,{}});return; }
        if (!signature(record)) {result.diagnostics.push_back({DiagnosticCode::EbrSignature,{}});return;}
        const auto values=entries(record);const auto logical=values.front();
        if(logical.type!=0 && logical.count!=0) {
            const auto first=current+logical.first; ++number;
            if(first<base || first>end || logical.count>end-first) result.diagnostics.push_back({DiagnosticCode::EbrLogicalOutside,{std::to_string(number)}});
            else add(media,first,logical.count,logical.type,number,result);
        } else if(logical.type!=0 || logical.count!=0) result.diagnostics.push_back({DiagnosticCode::EbrEmptyEntry,{}});
        const auto link=values[1];
        if(link.type==0 && link.count==0)return;
        if(!extended(link.type)||link.count==0) {result.diagnostics.push_back({DiagnosticCode::EbrInvalidLink,{}});return;}
        current=base+link.first;
        if(std::any_of(values.begin()+FirstExtraEbrEntry,values.end(),[](const auto& entry){return entry.type!=0||entry.count!=0;})) result.diagnostics.push_back({DiagnosticCode::EbrAdditionalEntries,{}});
    }
    if(current!=0)result.diagnostics.push_back({DiagnosticCode::EbrLimit,{}});
}
}
std::optional<VolumeDetectionResult> detectMbr(const BlockMedia& media, std::stop_token token) {
    checkCancellation(token);
    if(!media.isHardDisk() || media.blockSize()<BootRecordSize || media.capacity()<media.blockSize())return std::nullopt;
    const auto record=bootRecord(media,0,token);if(!signature(record))return std::nullopt;
    const auto values=entries(record);
    if(std::any_of(values.begin(),values.end(),[](const auto& entry){return entry.type==ProtectiveGptType;}))return std::nullopt;
    if(std::all_of(values.begin(),values.end(),[](const auto& entry){return entry.type==0&&entry.count==0;}))return std::nullopt;
    VolumeDetectionResult result;std::uint32_t number=0;
    for(const auto entry:values) {
        if(entry.type==0 && entry.count==0)continue;
        if(entry.type==0 || entry.count==0) {result.diagnostics.push_back({DiagnosticCode::MbrEmptyEntry,{}});continue;}
        if(extended(entry.type))chain(media,entry,number,result,token);else add(media,entry.first,entry.count,entry.type,++number,result);
    }
    Binary::overlaps(result,DiagnosticCode::MbrOverlap);return result;
}
}
