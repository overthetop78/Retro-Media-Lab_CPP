#include "Internal.h"
#include <algorithm>
#include <chrono>
#include <numeric>
namespace RetroMediaLab::MediaFileSystems::ProDos {
using namespace Constants;
namespace {
Timestamp utcNow(){
    using namespace std::chrono;const auto point=floor<seconds>(system_clock::now());const auto date=floor<days>(point);const year_month_day ymd(date);const hh_mm_ss time(point-date);
    return {int(ymd.year()),std::int32_t(unsigned(ymd.month())),std::int32_t(unsigned(ymd.day())),std::int32_t(time.hours().count()),std::int32_t(time.minutes().count()),std::int32_t(time.seconds().count())};
}
struct State {
    std::uint32_t count;std::vector<std::vector<std::uint8_t>> blocks;std::vector<bool> used;std::optional<Timestamp> defaultTime;std::stop_token token;
    State(std::uint32_t n,std::optional<Timestamp> time,std::stop_token stop):count(n),blocks(n,std::vector<std::uint8_t>(BlockSize)),used(n),defaultTime(time),token(stop){for(std::uint32_t i=0;i<=DefaultBitmapBlock;++i)used[i]=true;}
    Timestamp stamp() const{return defaultTime.value_or(utcNow());}
    std::vector<std::int32_t> allocate(std::uint32_t requested){
        checkCancellation(token);std::vector<std::int32_t> result;result.reserve(requested);
        for(std::uint32_t i=DefaultBitmapBlock+1;i<count&&result.size()<requested;++i)if(!used[i])result.push_back(std::int32_t(i));
        if(result.size()!=requested)throw MediaError(DiagnosticCode::ProDosDiskFull);for(auto number:result)used[number]=true;return result;
    }
};
void name(MutableBytes bytes,std::size_t offset,Storage type,std::string_view value){
    if(!validName(value))throw MediaError(DiagnosticCode::ProDosName,{std::string(value)});
    bytes[offset+StorageAndNameLengthOffset]=std::uint8_t(std::uint8_t(type)<<StorageTypeShift|value.size());std::copy(value.begin(),value.end(),bytes.begin()+offset+NameOffset);
}
void link(State& state,const std::vector<std::int32_t>& blocks){
    for(std::size_t i=0;i<blocks.size();++i){Internal::write16(state.blocks[blocks[i]],PreviousBlockOffset,i==0?0:blocks[i-1]);Internal::write16(state.blocks[blocks[i]],NextBlockOffset,i+1==blocks.size()?0:blocks[i+1]);}
}
void headerFields(State& state,MutableBytes block,std::string_view value,std::uint32_t files,bool root,std::int32_t parentBlock=0,std::int32_t parentEntry=0){
    name(block,HeaderOffset,root?Storage::VolumeHeader:Storage::SubdirectoryHeader,value);writeTime(block,HeaderOffset+CreatedDateOffset,state.stamp());block[HeaderOffset+AccessOffset]=DefaultAccess;
    block[HeaderEntryLengthOffset]=std::uint8_t(EntrySize);block[HeaderEntryLengthOffset+1]=std::uint8_t(EntriesPerDirectoryBlock);Internal::write16(block,HeaderEntryLengthOffset+sizeof(std::uint16_t),files);
    if(root){Internal::write16(block,BitmapBlockOffset,DefaultBitmapBlock);Internal::write16(block,TotalBlocksOffset,state.count);}
    else{block[SubdirectoryReservedOffset]=SubdirectoryReservedValue;block[SubdirectoryVersionOffset]=SubdirectoryVersion;Internal::write16(block,SubdirectoryParentBlockOffset,parentBlock);if(parentEntry<0||parentEntry>std::numeric_limits<std::uint8_t>::max())throw MediaError(DiagnosticCode::ProDosGeometry);block[SubdirectoryParentEntryOffset]=std::uint8_t(parentEntry);block[SubdirectoryParentEntryLengthOffset]=std::uint8_t(EntrySize);}
}
void entryFields(MutableBytes block,std::size_t offset,const MigrationEntry& entry,Storage storage,FileType type,std::int32_t key,std::uint32_t used,std::uint32_t length,std::int32_t parent){
    name(block,offset,storage,entry.targetName);block[offset+FileTypeOffset]=std::uint8_t(type);Internal::write16(block,offset+KeyBlockOffset,key);Internal::write16(block,offset+BlocksUsedOffset,used);Internal::write24(block,offset+EndOfFileOffset,length);
    writeTime(block,std::int64_t(offset+CreatedDateOffset),entry.modified);writeTime(block,std::int64_t(offset+ModifiedDateOffset),entry.modified);
    block[offset+AccessOffset]=entry.rawAttributes>0&&entry.rawAttributes<=std::numeric_limits<std::uint8_t>::max()?std::uint8_t(entry.rawAttributes):DefaultAccess;Internal::write16(block,offset+HeaderPointerOffset,parent);
}
struct Allocation {Storage storage;std::int32_t key;std::uint32_t used;};
Allocation storage(State& state,Bytes data){
    const auto count=std::max<std::uint32_t>(1,std::uint32_t((data.size()+BlockSize-1)/BlockSize));const auto blocks=state.allocate(count);
    for(std::uint32_t i=0;i<count;++i){checkCancellation(state.token);const auto start=std::size_t(i)*BlockSize;const auto available=start>=data.size()?0:std::min<std::size_t>(BlockSize,data.size()-start);if(available)std::copy_n(data.begin()+start,available,state.blocks[blocks[i]].begin());}
    if(count==1)return {Storage::Seedling,blocks.front(),1};
    if(count<=IndexPointerCount){const auto index=state.allocate(1).front();for(std::uint32_t i=0;i<count;++i)Internal::writeIndex(state.blocks[index],i,blocks[i]);return {Storage::Sapling,index,count+1};}
    const auto childrenCount=(count+IndexPointerCount-1)/IndexPointerCount;const auto master=state.allocate(1).front();const auto children=state.allocate(childrenCount);
    for(std::uint32_t child=0;child<childrenCount;++child){Internal::writeIndex(state.blocks[master],child,children[child]);const auto first=child*IndexPointerCount;for(std::uint32_t i=0;i<std::min(IndexPointerCount,count-first);++i)Internal::writeIndex(state.blocks[children[child]],i,blocks[first+i]);}
    return {Storage::Tree,master,count+childrenCount+1};
}
void entries(State& state,const std::vector<MigrationEntry>& values,const std::vector<std::int32_t>& directories,std::int32_t header){
    std::size_t slot=1;
    for(const auto& entry:values){
        checkCancellation(state.token);const auto blockIndex=slot/EntriesPerDirectoryBlock,entryIndex=slot%EntriesPerDirectoryBlock;
        if(blockIndex>=directories.size())throw MediaError(DiagnosticCode::ProDosDiskFull);const auto offset=HeaderOffset+entryIndex*EntrySize;
        if(entry.kind==EntryKind::Directory){
            if(entry.children.size()>std::numeric_limits<std::uint32_t>::max()-EntriesPerDirectoryBlock)throw MediaError(DiagnosticCode::ProDosDiskFull);
            const auto count=std::max<std::uint32_t>(1,std::uint32_t((entry.children.size()+1+EntriesPerDirectoryBlock-1)/EntriesPerDirectoryBlock));const auto blocks=state.allocate(count);link(state,blocks);
            headerFields(state,state.blocks[blocks.front()],entry.targetName,std::uint32_t(entry.children.size()),false,directories[blockIndex],std::int32_t(entryIndex+1));
            entries(state,entry.children,blocks,blocks.front());entryFields(state.blocks[directories[blockIndex]],offset,entry,Storage::Subdirectory,FileType::Directory,blocks.front(),count,count*BlockSize,header);
        }else if(entry.kind==EntryKind::File&&entry.content){
            if(entry.content->size()>MaximumFileLength)throw MediaError(DiagnosticCode::ProDosFileLength,{entry.sourcePath});const auto allocation=storage(state,*entry.content);
            entryFields(state.blocks[directories[blockIndex]],offset,entry,allocation.storage,FileType::Binary,allocation.key,allocation.used,std::uint32_t(entry.content->size()),header);
        }else throw MediaError(DiagnosticCode::ProDosEntry,{entry.sourcePath});++slot;
    }
}
}
SectorWritePlan write(const MigrationPlan& plan,const SectorGeometry& geometry,std::uint32_t count,std::span<const SectorAddress> addresses,std::optional<Timestamp> defaultTime,std::stop_token token){
    if(geometry.formatId!=ProDos&&geometry.formatId!=ProDos140&&geometry.formatId!=ProDos800)throw MediaError(DiagnosticCode::ProDosWriteFormat,{geometry.formatId});
    if(geometry.blockSize!=BlockSize||!geometry.cylinders||!geometry.heads||!geometry.sectorsPerTrack||count<=DefaultBitmapBlock||count>BlocksPerBitmapBlock||addresses.size()!=count)throw MediaError(DiagnosticCode::ProDosGeometry);
    checkCancellation(token);State state(count,defaultTime,token);
    if(plan.entries.size()>RootDirectoryBlockCount*EntriesPerDirectoryBlock-1)throw MediaError(DiagnosticCode::ProDosDiskFull);
    std::vector<std::int32_t> root(RootDirectoryBlockCount);std::iota(root.begin(),root.end(),std::int32_t(RootBlock));link(state,root);
    headerFields(state,state.blocks[RootBlock],plan.volumeName,std::uint32_t(plan.entries.size()),true);entries(state,plan.entries,root,RootBlock);
    auto& bitmap=state.blocks[DefaultBitmapBlock];for(std::uint32_t i=0;i<count;++i)if(!state.used[i])bitmap[i/BitsPerByte]|=std::uint8_t(BitmapHighBitMask>>(i&(BitsPerByte-1)));
    SectorWritePlan result;result.geometry=geometry;result.logicalBlockCount=count;result.blocks.reserve(count);for(std::uint32_t i=0;i<count;++i)result.blocks.push_back({i,addresses[i],std::move(state.blocks[i])});return result;
}
}
