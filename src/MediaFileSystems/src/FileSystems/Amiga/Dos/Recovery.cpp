#include <RetroMediaLab/MediaFileSystems/FileSystemDisplayNames.h>
#include "Internal.h"
#include <unordered_map>
#include <limits>
#include <functional>
namespace RetroMediaLab::MediaFileSystems::AmigaDos::Internal {
namespace {
struct Candidate {std::int64_t number,parent;std::string name;EntryKind kind;std::vector<std::uint8_t> data;};
FileSystemEntry recoveredEntry(const Candidate& c,std::vector<FileSystemEntry> children,std::optional<std::vector<std::uint8_t>> content,bool valid){
    FileSystemEntry entry;entry.name=c.name;entry.kind=c.kind;entry.size=c.kind==EntryKind::File?unsignedInteger(c.data,FileSizeOffset):0;
    entry.modified=readTime(c.data,DateOffset);entry.comment=readName(c.data,LongNameOffset,CommentMaximumLength);entry.rawAttributes=unsignedInteger(c.data,ProtectionOffset);
    entry.storageReference=c.number;entry.metadataValid=valid;entry.children=std::move(children);entry.content=std::move(content);return entry;
}
}
std::optional<FileSystemVolume> recover(const SectorImage& image,std::stop_token token){
    checkCancellation(token);if(image.geometry().blockSize!=BlockSize)return {};
    const auto boot=image.block(0,token);if(!boot||boot->data.size()!=BlockSize||!dosPrefix(boot->data)||boot->data[DosVariantOffset]>MaximumVariant)return {};
    const auto variant=Variant(boot->data[DosVariantOffset]);const auto declared=integer(boot->data,BootRootPointerOffset);
    const auto expected=declared>0&&std::uint64_t(declared)<image.blockCount()?std::int64_t(declared):std::int64_t(image.blockCount()/BootBlockCount);
    if(expected<=0||std::uint64_t(expected)>=image.blockCount())return {};
    const auto root=image.block(std::uint64_t(expected),token);
    if(root&&root->integrityValid!=false&&root->data.size()==BlockSize&&!unavailableBlock(root->data))return {};
    std::vector<Candidate> candidates;std::unordered_map<std::int64_t,std::size_t> byNumber;std::unordered_map<std::int64_t,std::vector<std::size_t>> byParent;
    image.visitAvailableBlocks([&](auto logical){
        checkCancellation(token);const auto block=image.block(logical,token);
        if(!block||block->integrityValid!=true||!recordedIntegrity(*block)||block->data.size()!=BlockSize)return true;
        const Bytes data=block->data;
        if(integer(data,PrimaryTypeOffset)!=HeaderPrimaryType||integer(data,HeaderKeyOffset)!=std::int64_t(logical)||!validChecksum(data))return true;
        const auto kind=entryKind(integer(data,SecondaryTypeOffset));if(kind!=EntryKind::Directory&&kind!=EntryKind::File)return true;
        const auto parent=integer(data,ParentBlockOffset);const auto name=readEntryName(data,variant);
        if(parent<=0||std::uint64_t(parent)>=image.blockCount()||!recoverableName(name))return true;
        byNumber[std::int64_t(logical)]=candidates.size();byParent[parent].push_back(candidates.size());candidates.push_back({std::int64_t(logical),parent,name,kind,block->data});return true;
    },token);
    if(!byParent.contains(expected))return {};
    FileSystemVolume volume;
    std::function<FileSystemEntry(std::size_t,std::set<std::int64_t>,std::uint32_t)> build;
    build=[&](std::size_t index,std::set<std::int64_t> ancestors,std::uint32_t depth){
        checkCancellation(token);const auto& c=candidates[index];
        if(!ancestors.insert(c.number).second)return recoveredEntry(c,{},std::nullopt,false);
        if(depth>MaximumDirectoryDepth){volume.warnings.push_back({DiagnosticCode::AmigaDepth,{std::to_string(depth)}});return recoveredEntry(c,{},std::nullopt,false);}
        std::vector<FileSystemEntry> children;
        if(c.kind==EntryKind::Directory)if(const auto it=byParent.find(c.number);it!=byParent.end())for(const auto child:it->second)children.push_back(build(child,ancestors,depth+1));
        sort(children);if(c.kind!=EntryKind::File)return recoveredEntry(c,std::move(children),std::nullopt,true);
        try{
            const auto size=unsignedInteger(c.data,FileSizeOffset);if(size>std::uint32_t(std::numeric_limits<std::int32_t>::max()))throw MediaError(DiagnosticCode::AmigaFileSize,{std::to_string(size)});
            auto contents=file(image,c.data,size,variant,volume.warnings,token);return recoveredEntry(c,std::move(children),std::move(contents.content),contents.valid);
        }catch(const MediaError& error){if(error.kind()!=ErrorKind::InvalidData&&error.kind()!=ErrorKind::Overflow&&error.kind()!=ErrorKind::ArgumentOutOfRange)throw;auto diagnostic=error.diagnostic();diagnostic.context.push_back(c.name);volume.warnings.push_back(std::move(diagnostic));return recoveredEntry(c,std::move(children),std::nullopt,false);}
    };
    for(std::size_t index=0;index<candidates.size();++index){
        const auto& c=candidates[index];const auto parent=byNumber.find(c.parent);
        if(c.parent==expected||parent==byNumber.end()||candidates[parent->second].kind!=EntryKind::Directory)volume.entries.push_back(build(index,{},0));
    }
    sort(volume.entries);volume.warnings.insert(volume.warnings.begin(),{DiagnosticCode::AmigaRecovered,{std::to_string(candidates.size())}});
    volume.fileSystemId=fileSystemId(variant);applyFileSystemDisplayName(volume,image.geometry());volume.capacity=image.capacity();volume.freeSpaceKnown=false;return volume;
}
}
