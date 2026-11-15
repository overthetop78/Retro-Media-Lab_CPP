#include "WriterInternal.h"
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::CommodoreDos::Writing {
using namespace Constants;
void Builder::dataChain(Bytes content,const std::vector<std::int32_t>& blocks){
    for(std::size_t index=0;index<blocks.size();++index){
        checkCancellation(token_);auto& sector=sectors_[blocks[index]];
        const auto used=std::min(std::size_t(DataBytes),content.size()-index*DataBytes);
        if(index+1<blocks.size())address(sector,0,blocks[index+1]);else sector[1]=std::uint8_t(used+1);
        std::copy_n(content.begin()+index*DataBytes,used,sector.begin()+LinkLength);
    }
}
std::vector<std::int32_t> Builder::relativeSides(const std::string& path,const std::vector<std::int32_t>& data,std::uint8_t length){
    const auto count=std::max(std::size_t(1),(data.size()+RelativePointersPerSector-1)/RelativePointersPerSector);
    if(count>MaximumRelativeSides)throw MediaError(DiagnosticCode::CbmWriteRelativeSize,{path});
    auto blocks=allocate(count);
    for(std::size_t index=0;index<blocks.size();++index){
        checkCancellation(token_);auto& sector=sectors_[blocks[index]];
        if(index+1<blocks.size())address(sector,0,blocks[index+1]);
        sector[RelativeNumberOffset]=std::uint8_t(index);sector[RelativeRecordOffset]=length;
        for(std::size_t side=0;side<blocks.size();++side)address(sector,RelativeTableOffset+side*AddressBytes,blocks[side]);
        const auto first=index*RelativePointersPerSector;
        const auto pointers=std::min(std::size_t(RelativePointersPerSector),data.size()-first);
        for(std::size_t pointer=0;pointer<pointers;++pointer)address(sector,RelativePointersOffset+pointer*AddressBytes,data[first+pointer]);
    }
    return blocks;
}
void Builder::writeFile(const MigrationEntry& entry,std::size_t index){
    checkCancellation(token_);
    const auto native=QString::fromStdString(plan_.sourceFileSystemId).compare(QString::fromUtf8(FileSystemId),Qt::CaseInsensitive)==0;
    const auto rawType=native?std::uint8_t(entry.rawAttributes):policy_.defaultFileType();
    const auto base=rawType&BaseTypeMask;
    if(native&&(base<Seq||base>Rel))throw MediaError(DiagnosticCode::CbmWriteEntry,{entry.sourcePath});
    const auto& content=*entry.content;
    const auto blocks=allocate((content.size()+DataBytes-1)/DataBytes);dataChain(content,blocks);
    const auto relative=base==Rel;
    const auto length=relative?(native?std::uint8_t(entry.rawAttributes>>RecordLengthShift):policy_.relativeRecordLength()):0;
    if(relative&&length==0)throw MediaError(DiagnosticCode::CbmWriteEntry,{entry.sourcePath});
    const auto sides=relative?relativeSides(entry.sourcePath,blocks,std::uint8_t(length)):std::vector<std::int32_t>{};
    auto& directory=sectors_[directories_[index/DirectorySlots]];const auto offset=index%DirectorySlots*DirectoryEntrySize;
    directory[offset+TypeOffset]=rawType;
    if(!blocks.empty())address(directory,offset+DataTrackOffset,blocks.front());
    const auto name=nameBytes(entry.targetName);std::copy(name.begin(),name.end(),directory.begin()+offset+NameOffset);
    if(!sides.empty())address(directory,offset+RelativeSideOffset,sides.front());
    if(relative)directory[offset+RecordLengthOffset]=std::uint8_t(length);
    const auto count=blocks.size()+sides.size();directory[offset+DeclaredBlocksOffset]=std::uint8_t(count);directory[offset+DeclaredBlocksOffset+1]=std::uint8_t(count>>BitsPerByte);
}
void Builder::writeFiles(){
    for(std::size_t index=0;index<plan_.entries.size();++index)writeFile(plan_.entries[index],index);
    for(std::size_t index=0;index+1<directories_.size();++index)address(sectors_[directories_[index]],0,directories_[index+1]);
}
}
