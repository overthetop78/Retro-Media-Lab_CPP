#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Ucsd::Internal {
using namespace Constants;
Directory directory(const SectorImage& image,const Header& header,const Blocks& data,std::vector<Diagnostic>& warnings,std::stop_token token){
    checkCancellation(token);Directory result;result.valid=data.valid();for(std::int32_t n=0;n<header.end;++n)result.used.insert(n);const auto capacity=data.bytes.size()>=EntrySize?(data.bytes.size()-EntrySize)/EntrySize:0;const auto maximum=std::min({std::size_t(header.count),std::size_t(MaximumFileCount),capacity});
    for(std::size_t index=0;index<maximum;++index){
        checkCancellation(token);const auto entryIndex=index+1,offset=entryIndex*EntrySize;const auto firstDirectory=offset/BlockSize,lastDirectory=(offset+EntrySize-1)/BlockSize;if(!data.present[firstDirectory]||!data.present[lastDirectory]){result.valid=false;continue;}
        const auto entry=Bytes(data.bytes).subspan(offset,EntrySize);const auto first=word(entry,EntryFirstBlockOffset,header.order),end=word(entry,EntryLastBlockOffset,header.order);if(first==0&&end==0)continue;auto decoded=name(entry.subspan(EntryNameOffset,EntryNameFieldLength),MaximumFileNameLength);
        if(decoded.empty()){warnings.push_back({DiagnosticCode::UcsdInvalidName,{std::to_string(entryIndex),decoded}});result.valid=false;continue;}
        if(end<=first||end>header.total||first<header.end){warnings.push_back({DiagnosticCode::UcsdRange,{std::to_string(entryIndex),decoded,std::to_string(first),std::to_string(end),std::to_string(header.total)}});result.valid=false;continue;}
        std::vector<std::int32_t> overlap;for(std::int32_t n=first;n<end;++n)if(result.used.contains(n))overlap.push_back(n);if(!overlap.empty()){warnings.push_back({DiagnosticCode::UcsdOverlap,{std::to_string(entryIndex),decoded,join(overlap)}});result.valid=false;}for(std::int32_t n=first;n<end;++n)result.used.insert(n);
        const auto lastBytes=word(entry,EntryLastBlockBytesOffset,header.order);auto file=content(image,first,end,lastBytes,decoded,warnings,token);const auto kind=FileKind(word(entry,EntryKindOffset,header.order)&FileKindMask);FileSystemEntry value;value.name=std::move(decoded);value.kind=EntryKind::File;value.size=file.size;value.modified=date(word(entry,EntryDateOffset,header.order));value.comment=kindName(kind);value.commentResourceKey=TypeKeys[kindIndex(kind)];value.commentTranslationDomain=TranslationDomain;value.storageReference=first;value.metadataValid=file.valid&&overlap.empty();value.content=std::move(file.bytes);result.entries.push_back(std::move(value));result.valid=result.valid&&file.valid;
    }
    if(std::size_t(header.count)!=result.entries.size())warnings.push_back({DiagnosticCode::UcsdCount,{std::to_string(header.count),std::to_string(result.entries.size())}});
    std::stable_sort(result.entries.begin(),result.entries.end(),[](const auto& a,const auto& b){return std::lexicographical_compare(a.name.begin(),a.name.end(),b.name.begin(),b.name.end(),[](auto x,auto y){const auto upper=[](std::uint8_t c){return c>=UpperA&&c<=UpperZ?std::uint8_t(c+LowerDelta):c;};return upper(std::uint8_t(x))<upper(std::uint8_t(y));});});return result;
}
}
