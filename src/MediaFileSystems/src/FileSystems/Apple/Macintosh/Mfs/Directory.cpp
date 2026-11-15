#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::MacMfs::Internal {
using namespace Constants;

std::vector<FileSystemEntry> directory(const SectorImage& image,std::int32_t start,std::int32_t count,const AllocationMap& map,std::int32_t allocationStart,std::uint32_t allocationSize,std::vector<Diagnostic>& warnings,std::stop_token token){
    std::vector<FileSystemEntry> entries;const auto end=std::min(std::int64_t(start)+count,std::int64_t(std::min(image.blockCount(),std::uint64_t(std::numeric_limits<std::int64_t>::max()))));
    if(start<0||count<0||std::uint64_t(start)>image.blockCount()||end<start)return entries;
    for(std::int64_t blockNumber=start;blockNumber<end;++blockNumber){
        checkCancellation(token);const auto block=readBlocks(image,std::int32_t(blockNumber),1,BlockContext::Directory,warnings,token);if(!block.valid)continue;const auto& bytes=block.bytes;std::size_t offset=0;
        while(offset+MinimumEntry<=bytes.size()&&(bytes[offset+Flags]&Active)!=0){
            const auto entryOffset=offset;const auto flags=bytes[offset+Flags],length=bytes[offset+NameLength];const auto finder=Bytes(bytes).subspan(offset+FinderInfo,FinderLength);
            const auto number=Binary::big<std::uint32_t>(bytes,offset+FileNumber),dataLength=Binary::big<std::uint32_t>(bytes,offset+DataLength),resourceLength=Binary::big<std::uint32_t>(bytes,offset+ResourceLength);
            const auto dataStart=Binary::big<std::uint16_t>(bytes,offset+DataStart),resourceStart=Binary::big<std::uint16_t>(bytes,offset+ResourceStart);
            const auto modified=Macintosh::fromSeconds(Binary::big<std::uint32_t>(bytes,offset+ModifiedDate));
            if(length>MaximumName||offset+Name+length>bytes.size()){warnings.push_back({DiagnosticCode::MfsDirectoryEntry,{std::to_string(blockNumber),std::to_string(entryOffset)}});break;}
            auto name=Macintosh::decodeName(Bytes(bytes).subspan(offset+Name,length));
            auto data=readFork(image,map,allocationStart,allocationSize,dataStart,dataLength,name,ForkKind::Data,warnings,token);auto resource=readFork(image,map,allocationStart,allocationSize,resourceStart,resourceLength,name,ForkKind::Resource,warnings,token);
            auto type=Macintosh::fileType(finder.first(FinderTypeLength));const bool fallback=type.empty();
            FileSystemEntry entry;entry.name=std::move(name);entry.kind=EntryKind::File;entry.size=std::uint64_t(dataLength)+resourceLength;entry.modified=modified;entry.comment=fallback?std::string(DefaultDescription):std::move(type);entry.rawAttributes=flags;entry.storageReference=Macintosh::storageReference(number);entry.metadataValid=data.valid&&resource.valid;entry.content=data.content.empty()?std::move(resource.content):std::move(data.content);
            if(fallback){entry.commentResourceKey=DefaultDescriptionKey;entry.commentTranslationDomain=TranslationDomain;}entries.push_back(std::move(entry));
            offset+=Name+length;if(offset%Alignment)++offset;if(offset<=entryOffset)break;
        }
    }
    std::stable_sort(entries.begin(),entries.end(),[](const auto& a,const auto& b){return Macintosh::lessName(a.name,b.name);});return entries;
}
}
