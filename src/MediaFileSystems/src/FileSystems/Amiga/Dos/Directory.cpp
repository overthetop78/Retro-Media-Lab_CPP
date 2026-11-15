#include "Internal.h"
#include <limits>
namespace RetroMediaLab::MediaFileSystems::AmigaDos::Internal {
std::vector<FileSystemEntry> directory(const SectorImage& image,Bytes data,int hashSize,Variant variant,std::set<std::int64_t>& visited,std::vector<Diagnostic>& warnings,std::uint32_t depth,std::stop_token token){
    checkCancellation(token);if(depth>MaximumDirectoryDepth){warnings.push_back({DiagnosticCode::AmigaDepth,{std::to_string(depth)}});return {};}
    std::vector<FileSystemEntry> result;
    for(int index=0;index<hashSize;++index){
        auto number=std::int64_t(integer(data,DataPointersOffset+index*WordSize));std::set<std::int64_t> chain;
        while(number!=0){
            checkCancellation(token);
            if(number<0||std::uint64_t(number)>=image.blockCount()||!chain.insert(number).second){warnings.push_back({DiagnosticCode::AmigaDirectoryChain,{std::to_string(number)}});break;}
            const auto block=image.block(std::uint64_t(number),token);
            if(!block){warnings.push_back({DiagnosticCode::AmigaMissingDirectoryEntry,{std::to_string(number)}});break;}
            if(block->data.size()!=BlockSize){warnings.push_back({DiagnosticCode::AmigaShortBlock,{std::to_string(number),std::to_string(block->data.size())}});break;}
            const Bytes bytes=block->data;const auto next=integer(bytes,HashChainOffset);
            if(!visited.insert(number).second){number=next;continue;}
            FileSystemEntry entry;entry.kind=entryKind(integer(bytes,SecondaryTypeOffset));entry.name=readEntryName(bytes,variant);
            if(entry.kind==EntryKind::Directory)entry.children=directory(image,bytes,RootHashTableEntryCount,variant,visited,warnings,depth+1,token);
            entry.size=entry.kind==EntryKind::File?unsignedInteger(bytes,FileSizeOffset):0;
            entry.metadataValid=validChecksum(bytes)&&recordedIntegrity(*block);
            if(!recordedIntegrity(*block)){Diagnostic d{DiagnosticCode::AmigaRecordedIntegrity,{std::to_string(number)}};warnings.push_back(d);entry.diagnostics.push_back(std::move(d));}
            if(entry.kind==EntryKind::File){
                try{
                    if(entry.size>std::uint64_t(std::numeric_limits<std::int32_t>::max()))throw MediaError(DiagnosticCode::AmigaFileSize,{std::to_string(entry.size)});
                    auto contents=file(image,bytes,std::uint32_t(entry.size),variant,warnings,token);entry.content=std::move(contents.content);entry.dataValid=contents.valid;
                }catch(const MediaError& error){if(error.kind()!=ErrorKind::InvalidData&&error.kind()!=ErrorKind::Overflow&&error.kind()!=ErrorKind::ArgumentOutOfRange)throw;auto d=error.diagnostic();d.context.push_back(entry.name);warnings.push_back(d);entry.diagnostics.push_back(std::move(d));entry.dataValid=false;}
            }
            entry.modified=readTime(bytes,DateOffset);entry.comment=readName(bytes,LongNameOffset,CommentMaximumLength);entry.rawAttributes=unsignedInteger(bytes,ProtectionOffset);entry.storageReference=number;
            result.push_back(std::move(entry));number=next;
        }
    }
    sort(result);return result;
}
}
