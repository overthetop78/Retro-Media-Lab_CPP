#include "Structures.h"
#include "../../../Primitives/Crc32C.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QUuid>
#include <map>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vhdx {
Guid readGuid(std::span<const std::uint8_t> bytes){Guid id{};std::copy_n(bytes.begin(),id.size(),id.begin());return id;}
Guid randomGuid(){using namespace Constants;const auto bytes=QUuid::createUuid().toRfc4122();auto id=readGuid({reinterpret_cast<const std::uint8_t*>(bytes.constData()),GuidBytes});qToLittleEndian(qFromBigEndian<std::uint32_t>(id.data()),id.data());qToLittleEndian(qFromBigEndian<std::uint16_t>(id.data()+GuidMiddleOffset),id.data()+GuidMiddleOffset);qToLittleEndian(qFromBigEndian<std::uint16_t>(id.data()+GuidLastOffset),id.data()+GuidLastOffset);return id;}
std::string guidText(const Guid& id){using namespace Constants;auto bytes=id;qToBigEndian(qFromLittleEndian<std::uint32_t>(bytes.data()),bytes.data());qToBigEndian(qFromLittleEndian<std::uint16_t>(bytes.data()+GuidMiddleOffset),bytes.data()+GuidMiddleOffset);qToBigEndian(qFromLittleEndian<std::uint16_t>(bytes.data()+GuidLastOffset),bytes.data()+GuidLastOffset);return QUuid::fromRfc4122(QByteArray(reinterpret_cast<const char*>(bytes.data()),GuidBytes)).toString(QUuid::WithoutBraces).toStdString();}
bool validCrc(std::span<const std::uint8_t> bytes){using namespace Constants;const auto expected=qFromLittleEndian<std::uint32_t>(bytes.data()+ChecksumOffset);std::vector<std::uint8_t> copy(bytes.begin(),bytes.end());std::fill_n(copy.begin()+ChecksumOffset,ChecksumSize,0);return Primitives::crc32c(copy)==expected;}
void writeCrc(std::span<std::uint8_t> bytes){using namespace Constants;std::fill_n(bytes.begin()+ChecksumOffset,ChecksumSize,0);qToLittleEndian(Primitives::crc32c(bytes),bytes.data()+ChecksumOffset);}
namespace {
struct Header{std::uint64_t sequence;Guid log;};
std::optional<Header> readHeader(const RecognitionContext& context,std::int64_t offset,std::stop_token token){using namespace Constants;const auto bytes=context.readAsync(offset,HeaderSize,token).get();if(!std::equal(HeaderSignature.begin(),HeaderSignature.end(),bytes.begin())||!validCrc(bytes)||qFromLittleEndian<std::uint16_t>(bytes.data()+HeaderVersionOffset)!=Version)return {};return Header{qFromLittleEndian<std::uint64_t>(bytes.data()+SequenceOffset),readGuid(std::span(bytes).subspan(LogIdOffset,GuidBytes))};}
std::optional<std::map<Guid,Region>> readRegions(const RecognitionContext& context,std::int64_t offset,std::stop_token token){
    using namespace Constants;const auto bytes=context.readAsync(offset,RegionTableSize,token).get();if(!std::equal(RegionSignature.begin(),RegionSignature.end(),bytes.begin())||!validCrc(bytes))return {};const auto count=qFromLittleEndian<std::uint32_t>(bytes.data()+RegionCountOffset);if(count>MaximumEntries)return {};std::map<Guid,Region> regions;
    for(std::uint32_t index=0;index<count;++index){checkCancellation(token);const auto entry=std::span(bytes).subspan(RegionEntriesOffset+std::size_t(index)*StructureEntrySize,StructureEntrySize);const auto id=readGuid(entry);const auto address=qFromLittleEndian<std::uint64_t>(entry.data()+RegionFileOffset);const auto length=qFromLittleEndian<std::uint32_t>(entry.data()+RegionLengthOffset),flags=qFromLittleEndian<std::uint32_t>(entry.data()+RegionFlagsOffset);
        if(address>MaximumSigned||!length||address%MiB||address>std::uint64_t(context.length())||length>std::uint64_t(context.length())-address)return {};
        if(id!=BatGuid&&id!=MetadataGuid){if(flags&RegionRequired)throw MediaError(DiagnosticCode::VhdxRequiredRegion,{guidText(id)});continue;}
        if(!regions.emplace(id,Region{std::int64_t(address),length}).second)return {};
    }return regions;
}
}
Structures readStructures(const RecognitionContext& context,std::stop_token token){
    using namespace Constants;checkCancellation(token);if(context.length()<Region2Offset+RegionTableSize)throw MediaError(DiagnosticCode::VhdxShort);const auto signature=context.readAsync(0,std::int32_t(FileSignature.size()),token).get();if(!std::equal(signature.begin(),signature.end(),FileSignature.begin(),FileSignature.end()))throw MediaError(DiagnosticCode::VhdxSignature);
    const auto first=readHeader(context,Header1Offset,token),second=readHeader(context,Header2Offset,token);if(!first&&!second)throw MediaError(DiagnosticCode::VhdxHeaders);const auto header=!first?*second:!second?*first:first->sequence>=second->sequence?*first:*second;if(header.log!=Guid{})throw MediaError(DiagnosticCode::VhdxActiveLog);
    const auto region1=readRegions(context,Region1Offset,token),region2=readRegions(context,Region2Offset,token);if(!region1&&!region2)throw MediaError(DiagnosticCode::VhdxRegions);const auto& regions=region1?*region1:*region2;const auto bat=regions.find(BatGuid),metadata=regions.find(MetadataGuid);if(bat==regions.end()||metadata==regions.end())throw MediaError(DiagnosticCode::VhdxMissingRegions);
    const auto values=readMetadata(context,metadata->second,token);if(values.fileFlags&HasParent)throw MediaError(DiagnosticCode::VhdxParent);return {bat->second,values};
}
}
