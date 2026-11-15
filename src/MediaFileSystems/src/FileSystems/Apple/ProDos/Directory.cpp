#include "Internal.h"
#include "../../../Primitives/Identifiers.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::ProDos::Internal {
using namespace Constants;
namespace {
void entry(const SectorImage& image,Bytes bytes,std::size_t offset,std::vector<Diagnostic>& warnings,std::set<std::int32_t>& visited,std::uint32_t depth,std::vector<FileSystemEntry>& entries,std::stop_token token){
    const auto storage=Storage(bytes[offset]>>StorageTypeShift);const auto length=bytes[offset]&NameLengthMask;
    if(storage==Storage::Inactive||!length||length>MaximumNameLength)return;
    FileSystemEntry result;result.name=readName(bytes,std::int64_t(offset));
    const auto key=Binary::little<std::uint16_t>(bytes,offset+KeyBlockOffset);const auto eof=read24(bytes,offset+EndOfFileOffset);
    result.modified=readTime(bytes,std::int64_t(offset+ModifiedDateOffset));setType(result,bytes[offset+FileTypeOffset]);result.rawAttributes=bytes[offset+AccessOffset];result.storageReference=key;
    if(storage==Storage::Subdirectory){auto children=directory(image,key,warnings,visited,depth+1,token);result.kind=EntryKind::Directory;result.metadataValid=children.valid;result.children=std::move(children.entries);entries.push_back(std::move(result));}
    else if(storage==Storage::Seedling||storage==Storage::Sapling||storage==Storage::Tree){auto data=content(image,storage,key,eof,result.name,warnings,token);result.kind=EntryKind::File;result.size=eof;result.metadataValid=data.valid;result.content=std::move(data.data);entries.push_back(std::move(result));}
}
}
Directory directory(const SectorImage& image,std::int32_t firstBlock,std::vector<Diagnostic>& warnings,std::set<std::int32_t>& visited,std::uint32_t depth,std::stop_token token){
    if(depth>MaximumDirectoryDepth){warnings.push_back({DiagnosticCode::ProDosDepth,{std::to_string(depth),std::to_string(firstBlock)}});return {{},false};}
    Directory result;auto number=firstBlock;std::set<std::int32_t> chain;bool first=true;
    while(number!=0){
        checkCancellation(token);const bool cyclic=!chain.insert(number).second,reused=!cyclic&&!visited.insert(number).second;
        const auto block=!cyclic&&!reused&&number>=0&&std::uint64_t(number)<image.blockCount()?image.block(std::uint64_t(number),token):nullptr;
        if(!block||block->data.size()!=BlockSize){warnings.push_back({DiagnosticCode::ProDosDirectory,{std::to_string(number),std::string(cyclic?True:False),std::string(reused?True:False)}});result.valid=false;break;}
        const auto start=first?FirstVolumeEntryOffset:FirstChainedEntryOffset;
        for(auto offset=start;offset+EntrySize<=block->data.size();offset+=EntrySize)entry(image,block->data,offset,warnings,visited,depth,result.entries,token);
        number=Binary::little<std::uint16_t>(block->data,NextBlockOffset);first=false;
    }
    std::stable_sort(result.entries.begin(),result.entries.end(),[](const auto& left,const auto& right){
        if((left.kind==EntryKind::Directory)!=(right.kind==EntryKind::Directory))return left.kind==EntryKind::Directory;
        const auto a=Identifiers::utf8(left.name),b=Identifiers::utf8(right.name);return QString::compare(a,b,Qt::CaseInsensitive)<0;
    });return result;
}
}
