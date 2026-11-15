#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Rt11::Internal {
using namespace Constants;
Directory directory(const SectorImage& image,std::int32_t firstDirectory,std::stop_token token){
    Directory result;std::set<std::int32_t> seen;std::int32_t segment=1;
    while(segment!=0){
        checkCancellation(token);if(segment<1||segment>MaximumSegmentCount){result.warnings.push_back({DiagnosticCode::Rt11SegmentRange,{std::to_string(segment),std::to_string(firstDirectory)}});result.valid=false;break;}
        if(!seen.insert(segment).second){result.warnings.push_back({DiagnosticCode::Rt11SegmentCycle,{std::to_string(segment),std::to_string(firstDirectory)}});result.valid=false;break;}
        const auto firstLong=std::int64_t(firstDirectory)+(std::int64_t(segment)-1)*SegmentBlockCount;
        if(firstLong<0||std::uint64_t(firstLong+1)>=image.blockCount()){const auto clamped=std::clamp(firstLong,std::int64_t(std::numeric_limits<std::int32_t>::min()),std::int64_t(std::numeric_limits<std::int32_t>::max()));result.warnings.push_back({DiagnosticCode::Rt11SegmentOutside,{std::to_string(segment),std::to_string(clamped)}});result.valid=false;break;}
        const auto first=std::int32_t(firstLong);const auto block=pair(image,first,token);if(!block.valid()){result.warnings.push_back({DiagnosticCode::Rt11SegmentMissing,{std::to_string(segment),std::to_string(first)}});result.valid=false;break;}
        const auto bytes=Bytes(block.bytes);const auto entrySize=MinimumEntrySize+Binary::little<std::uint16_t>(bytes,ExtraBytesOffset);if(entrySize<MinimumEntrySize||entrySize>MaximumEntrySize){result.warnings.push_back({DiagnosticCode::Rt11EntrySize,{std::to_string(segment),std::to_string(entrySize)}});result.valid=false;break;}
        std::int32_t dataBlock=Binary::little<std::uint16_t>(bytes,DataBlockOffset);
        for(std::size_t offset=EntriesOffset;offset+entrySize<=bytes.size();offset+=entrySize){
            checkCancellation(token);const auto status=Binary::little<std::uint16_t>(bytes,offset+StatusOffset);if(has(status,EntryStatus::EndOfSegment))break;const auto length=Binary::little<std::uint16_t>(bytes,offset+BlockLengthOffset);
            if(has(status,EntryStatus::Empty)){result.free+=length;dataBlock+=length;continue;}
            if((status&(std::uint16_t(EntryStatus::Permanent)|std::uint16_t(EntryStatus::Tentative)))==0){dataBlock+=length;continue;}
            auto name=radix50(Binary::little<std::uint16_t>(bytes,offset+NameOffset))+radix50(Binary::little<std::uint16_t>(bytes,offset+NameOffset+sizeof(std::uint16_t)));while(!name.empty()&&name.back()==Space)name.pop_back();auto extension=radix50(Binary::little<std::uint16_t>(bytes,offset+ExtensionOffset));while(!extension.empty()&&extension.back()==Space)extension.pop_back();if(!extension.empty()){name+=ExtensionSeparator;name+=extension;}
            if(name.empty()){result.warnings.push_back({DiagnosticCode::Rt11EmptyName,{std::to_string(dataBlock),std::to_string(offset)}});dataBlock+=length;continue;}
            auto file=content(image,dataBlock,length,token);if(!file.valid){result.warnings.push_back({DiagnosticCode::Rt11MissingContent,{name,join(file.missing)}});result.valid=false;}
            FileSystemEntry entry;entry.name=std::move(name);entry.kind=EntryKind::File;entry.size=std::uint64_t(length)*BlockSize;entry.modified=decodeDate(Binary::little<std::uint16_t>(bytes,offset+DateOffset));const bool tentative=has(status,EntryStatus::Tentative);entry.comment=tentative?TentativeFileDescription:PermanentFileDescription;entry.commentResourceKey=tentative?TentativeKey:PermanentKey;entry.commentTranslationDomain=TranslationDomain;entry.rawAttributes=has(status,EntryStatus::Protected)?ProtectedAttribute:UnprotectedAttribute;entry.storageReference=dataBlock;entry.metadataValid=file.valid;entry.content=std::move(file.bytes);result.entries.push_back(std::move(entry));dataBlock+=length;
        }
        segment=Binary::little<std::uint16_t>(bytes,NextSegmentOffset);
    }
    if(!result.valid)result.free=0;std::stable_sort(result.entries.begin(),result.entries.end(),[](const auto& a,const auto& b){return a.name<b.name;});return result;
}
}
