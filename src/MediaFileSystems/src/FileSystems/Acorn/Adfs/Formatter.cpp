#include <RetroMediaLab/MediaFileSystems/Adfs.h>
#include "Constants.h"
#include "../../../Primitives/Binary.h"
#include <algorithm>
#include <bit>
namespace RetroMediaLab::MediaFileSystems::Adfs {
namespace {
using namespace Constants;
std::uint8_t checksum(Bytes bytes){
    std::uint32_t sum=0,carry=0;
    for(int index=ChecksumStartOffset;index>=0;--index){const auto next=sum+carry+bytes[index];sum=next&ByteMask;carry=next>>BitsPerByte;}
    return std::uint8_t(sum);
}
void mapFields(MutableBytes bytes,std::uint32_t firstFree,std::uint32_t units){
    Binary::writeLittle24(bytes,0,firstFree);Binary::writeLittle24(bytes,UnitSize,units-firstFree);Binary::writeLittle24(bytes,TotalUnitsOffset,units);bytes[FreeEndOffset]=UInt24Size;
    std::fill_n(bytes.begin()+FirstNameOffset,HalfNameLength,std::uint8_t(Space));std::fill_n(bytes.begin()+SecondNameOffset,HalfNameLength,std::uint8_t(Space));
    bytes[UnitSize-1]=checksum(bytes.first(UnitSize));bytes[MapByteCount-1]=checksum(bytes.subspan(UnitSize,UnitSize));
}
std::uint8_t directoryCheck(Bytes bytes){
    std::uint32_t value=0;int offset=0;
    while(offset+WordBytes<=EntriesOffset){value=std::rotr(value,CheckRotation)^Binary::little<std::uint32_t>(bytes,offset);offset+=WordBytes;}
    while(offset<EntriesOffset)value=std::rotr(value,CheckRotation)^bytes[offset++];
    offset=TailOffset;while(offset%WordBytes!=0)value=std::rotr(value,CheckRotation)^bytes[offset++];
    while(offset<DirectorySize-WordBytes){value=std::rotr(value,CheckRotation)^Binary::little<std::uint32_t>(bytes,offset);offset+=WordBytes;}
    return std::uint8_t(value^(value>>BitsPerByte)^(value>>CheckSecondByteShift)^(value>>(BitsPerByte*UInt24Size)));
}
}
std::vector<std::uint8_t> createBbcHeader(std::uint32_t count){
    if(count<FirstFreeSector||count>UInt24Maximum)throw MediaError(DiagnosticCode::AdfsSectorCount,{std::to_string(count)});
    std::vector<std::uint8_t> bytes(FirstFreeSector*UnitSize);mapFields(bytes,FirstFreeSector,count);
    const auto root=BbcRootAddress*UnitSize;
    std::copy(Hugo.begin(),Hugo.end(),bytes.begin()+root+1);std::copy(Hugo.begin(),Hugo.end(),bytes.begin()+root+BbcDirectorySize-FooterOffset);
    bytes[root+BbcNameOffset]=RootName;bytes[root+BbcTitleOffset]=RootName;Binary::writeLittle24(bytes,root+BbcParentOffset,BbcRootAddress);return bytes;
}
std::vector<std::uint8_t> createDHeader(){
    std::vector<std::uint8_t> bytes(DRootAddress*UnitSize+DirectorySize);mapFields(bytes,DFirstFreeUnit,DBlockCount*BlockSize/UnitSize);
    auto directory=MutableBytes(bytes).subspan(DRootAddress*UnitSize,DirectorySize);
    std::copy(Nick.begin(),Nick.end(),directory.begin()+1);std::copy(Nick.begin(),Nick.end(),directory.begin()+DirectorySize-FooterOffset);
    directory[TitleOffset]=RootName;directory[DirectoryNameOffset]=RootName;Binary::writeLittle24(directory,ParentOffset,DRootAddress);
    directory.back()=directoryCheck(directory);return bytes;
}
}
