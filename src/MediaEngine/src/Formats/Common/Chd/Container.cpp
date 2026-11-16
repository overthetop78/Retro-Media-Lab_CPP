#include "Container.h"
#include "../../../Codecs/Crc16.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::Chd {
using namespace Constants;
Container::Container(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token):source_(std::move(source)){
    if(!source_)throw MediaError(DiagnosticCode::ChdRange);readHeader(token);
}
void Container::readAt(std::uint64_t offset,std::span<std::uint8_t> output,std::stop_token token)const{
    checkCancellation(token);if(offset>source_->length()||output.size()>source_->length()-offset)throw MediaError(DiagnosticCode::ChdRange);source_->readExactly(offset,output,token);
}
void Container::readHeader(std::stop_token token){
    std::array<std::uint8_t,HeaderBytes> bytes{};readAt(0,bytes,token);
    const auto u32=[&](int o){return qFromBigEndian<std::uint32_t>(bytes.data()+o);};const auto u64=[&](int o){return qFromBigEndian<std::uint64_t>(bytes.data()+o);};
    version_=u32(VersionOffset);if(!std::equal(Signature.begin(),Signature.end(),bytes.begin())||version_<Version1||version_>Version5||u32(HeaderSizeOffset)!=std::uint32_t(HeaderSizes[version_]))throw MediaError(DiagnosticCode::ChdHeader);
    std::uint64_t mapOffset=HeaderSizes[version_],count=0,hunk=0;
    if(version_==Version5){
        for(int i=0;i<CodecCount;++i)codecs_[i]=u32(CodecOffset+i*WordBytes);
        logicalBytes_=u64(LogicalOffset);mapOffset=u64(MapOffset);metadataOffset_=u64(MetadataOffset);hunk=u32(HunkOffset);unitBytes_=u32(UnitOffset);
        if(!hunk||!unitBytes_)throw MediaError(DiagnosticCode::ChdDimensions);
        count=logicalBytes_/hunk+(logicalBytes_%hunk!=0);
        if(std::any_of(bytes.begin()+ParentOffset,bytes.begin()+ParentOffset+ParentBytes,[](auto value){return value!=0;}))throw MediaError(DiagnosticCode::ChdParent);
    }else{
        codecs_[0]=u32(LegacyCodecOffset);if(codecs_[0]>MaximumLegacyCodec)throw MediaError(DiagnosticCode::ChdCodec);
        if(u32(LegacyFlagsOffset)&ParentFlag)throw MediaError(DiagnosticCode::ChdParent);
        count=u32(version_<Version3?OldCountOffset:LegacyCountOffset);
        if(version_<Version3){
            const auto sector=version_==Version1?SectorBytes:u32(OldSectorOffset);hunk=std::uint64_t(sector)*u32(OldSectorsOffset);
            logicalBytes_=sector;
            for(const auto factor:{u32(OldCylindersOffset),u32(OldHeadsOffset),u32(OldTracksOffset)}){if(factor&&logicalBytes_>MaximumLogical/factor)throw MediaError(DiagnosticCode::ChdDimensions);logicalBytes_*=factor;}
        }else{logicalBytes_=u64(LegacyLogicalOffset);metadataOffset_=u64(LegacyMetadataOffset);hunk=u32(version_==Version3?V3HunkOffset:V4HunkOffset);}
        unitBytes_=std::uint32_t(hunk);
    }
    if(logicalBytes_>MaximumLogical||!hunk||hunk>CodecConstants::MaximumHunk||!count||count>MaximumMap)throw MediaError(DiagnosticCode::ChdDimensions);
    hunkBytes_=std::uint32_t(hunk);hunkCount_=std::uint32_t(count);compressed_.resize(hunkBytes_);
    if(version_==Version5)readMap5(mapOffset,token);else readLegacyMap(mapOffset,token);initializeCodecs();
}
std::string Container::compression()const{
    std::string result;for(auto codec:codecs_){if(!codec)continue;if(!result.empty())result+=CodecSeparator;if(version_<Version5){for(auto c:LegacyCodecName)result+=c;}else for(int shift=LongBits;shift>=0;shift-=ByteBits)result+=char((codec>>shift)&CodecConstants::ByteMask);}return result;
}
std::optional<std::string> Container::metadata(std::string_view tag,int index,std::stop_token token)const{
    if(tag.size()!=TagBytes||index<0)throw MediaError(DiagnosticCode::ChdMetadata);std::scoped_lock lock(mutex_);std::uint32_t expected=0;for(unsigned char value:tag)expected=(expected<<ByteBits)|value;
    auto offset=metadataOffset_,remaining=source_->length()/MetadataHeaderBytes+1;std::array<std::uint8_t,MetadataHeaderBytes> header{};
    while(offset&&remaining--){readAt(offset,header,token);if(qFromBigEndian<std::uint32_t>(header.data()+MetadataTagOffset)==expected&&index--==0){
            std::vector<std::uint8_t> data(qFromBigEndian<std::uint32_t>(header.data()+MetadataLengthOffset)&MetadataLengthMask);readAt(offset+MetadataHeaderBytes,data,token);std::string result;
            for(auto value:data){if(!value)break;result+=value<AsciiLimit?char(value):AsciiReplacement;}return result;
        }offset=qFromBigEndian<std::uint64_t>(header.data()+MetadataNextOffset);}
    if(offset)throw MediaError(DiagnosticCode::ChdMetadata);return {};
}
std::span<const std::uint8_t> Container::readCompressed(std::uint64_t offset,std::uint32_t length,std::stop_token token){
    if(length>compressed_.size())throw MediaError(DiagnosticCode::ChdMap);auto output=std::span(compressed_).first(length);readAt(offset,output,token);return output;
}
void Container::readHunk(std::uint32_t number,std::span<std::uint8_t> output,std::stop_token token){
    if(output.size()<hunkBytes_)throw MediaError(DiagnosticCode::ChdHunkBuffer);output=output.first(hunkBytes_);std::scoped_lock lock(mutex_);auto remaining=hunkCount_;
    for(;;){checkCancellation(token);if(number>=hunkCount_||!remaining--)throw MediaError(DiagnosticCode::ChdHunkReference);
        if(version_<Version5){const auto& entry=legacyMap_[number];switch(entry.type){
            case LegacyCompressed:decompress(codecs_[0],readCompressed(entry.offset,entry.length,token),output,token);return;
            case LegacyRaw:readAt(entry.offset,output,token);return;
            case LegacyMini:{std::array<std::uint8_t,PatternBytes> pattern{};qToBigEndian(entry.offset,pattern.data());for(std::size_t i=0;i<output.size();++i){if(!(i&CodecConstants::PollMask))checkCancellation(token);output[i]=pattern[i&PatternMask];}return;}
            case LegacySelf:if(entry.offset>=hunkCount_)throw MediaError(DiagnosticCode::ChdHunkReference);number=std::uint32_t(entry.offset);continue;
            default:throw MediaError(DiagnosticCode::ChdMap);}}
        if(!codecs_[0]){const auto offset=std::uint64_t(qFromBigEndian<std::uint32_t>(map_.data()+std::size_t(number)*UncompressedMapBytes))*hunkBytes_;if(!offset)std::fill(output.begin(),output.end(),0);else readAt(offset,output,token);return;}
        const auto* entry=map_.data()+std::size_t(number)*MapEntryBytes;const auto offset=(std::uint64_t(qFromBigEndian<std::uint16_t>(entry+EntryDataOffset))<<CodecConstants::WordBits)|qFromBigEndian<std::uint32_t>(entry+EntryDataOffset+sizeof(std::uint16_t));
        const auto type=entry[0];if(type==SelfType){if(offset>=hunkCount_)throw MediaError(DiagnosticCode::ChdHunkReference);number=std::uint32_t(offset);continue;}
        if(type<=CodecLast){const auto length=(std::uint32_t(entry[EntryLengthOffset])<<WordBits)|qFromBigEndian<std::uint16_t>(entry+EntryLengthOffset+1);decompress(codecs_[type],readCompressed(offset,length,token),output,token);}
        else if(type==RawType)readAt(offset,output,token);else throw MediaError(DiagnosticCode::ChdMap);
        if(Crc16::compute(output,Crc16::Constants::Ccitt,Crc16::Constants::AllBitsSet,token)!=qFromBigEndian<std::uint16_t>(entry+EntryCrcOffset))throw MediaError(DiagnosticCode::ChdHunkCrc);return;
    }
}
}
