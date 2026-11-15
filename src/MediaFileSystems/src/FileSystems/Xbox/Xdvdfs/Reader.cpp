#include "Constants.h"
#include "../../Optical/Common.h"
#include "../../../Primitives/Binary.h"
#include <RetroMediaLab/MediaFileSystems/OpticalReaders.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaFileSystems {
namespace Xdv {
using namespace XdvdfsConstants;
struct Context {std::shared_ptr<const OpticalTrack> track;std::uint32_t rootSector,rootSize;};
std::vector<std::uint8_t> sector(const OpticalTrack& track,std::int64_t absolute,std::stop_token token){if(absolute<track.firstSector()||absolute-track.firstSector()>=track.sectorCount())throw MediaError(DiagnosticCode::XdvSectorOutside);return Optical::sector(track,absolute-track.firstSector(),token);}
void validateExtent(const OpticalTrack& track,std::uint32_t start,std::uint32_t length){if(!length)return;const auto sectors=(std::uint64_t(length)+SectorSize-1)/SectorSize;const auto relative=std::int64_t(start)-track.firstSector();if(relative<0||relative>track.sectorCount()||sectors>std::uint64_t(track.sectorCount()-relative))throw MediaError(DiagnosticCode::XdvExtentOutside);}
std::vector<std::uint8_t> extent(const OpticalTrack& track,std::uint32_t start,std::uint32_t length,std::stop_token token){
    if(!length)return {};validateExtent(track,start,length);static_cast<void>(Optical::integer(length));
    std::vector<std::uint8_t> bytes(length);std::size_t offset=0;const auto count=(std::uint64_t(length)+SectorSize-1)/SectorSize;
    for(std::uint64_t index=0;index<count;++index){const auto data=sector(track,Optical::add(start,std::uint32_t(index)),token);const auto copied=std::min(data.size(),bytes.size()-offset);std::copy_n(data.begin(),copied,bytes.begin()+std::ptrdiff_t(offset));offset+=copied;}
    return bytes;
}
Context context(const ImageDocument& document,const VolumeDescriptor& volume,std::stop_token token){
    const auto& format=document.formatId();if(QString::fromUtf8(format).compare(QString::fromUtf8(Formats::MicrosoftXboxXdvdfs.data(),qsizetype(Formats::MicrosoftXboxXdvdfs.size())),Qt::CaseInsensitive)!=0)throw MediaError(DiagnosticCode::XdvNotImage);
    const auto track=Optical::resolve(document,volume,DiagnosticCode::XdvTrackRequired,DiagnosticCode::XdvNoTrack);
    if(track->userDataLength()!=SectorSize)throw MediaError(DiagnosticCode::XdvSectorSize);
    const auto descriptor=sector(*track,VolumeDescriptorSector,token);
    if(Optical::ascii(Optical::slice(descriptor,MagicOffset,MagicText.size()))!=MagicText||Optical::ascii(Optical::slice(descriptor,DescriptorTrailerOffset,MagicText.size()))!=MagicText)throw MediaError(DiagnosticCode::XdvDescriptor);
    const auto root=Binary::little<std::uint32_t>(descriptor,RootDirectorySectorOffset),size=Binary::little<std::uint32_t>(descriptor,RootDirectorySizeOffset);
    if(size%SectorSize)throw MediaError(DiagnosticCode::XdvRootAlignment);
    if(size>std::uint32_t(std::numeric_limits<int>::max()))throw MediaError(DiagnosticCode::XdvRootLarge);
    validateExtent(*track,root,size);return {track,root,size};
}
std::vector<FileSystemEntry> directory(const Context&,std::uint32_t,std::uint32_t,std::vector<Diagnostic>&,std::set<std::uint32_t>&,int,std::stop_token);
void node(const Context& ctx,Bytes table,std::size_t offset,int depth,std::set<std::size_t>& visited,std::vector<FileSystemEntry>& entries,std::vector<Diagnostic>& warnings,std::set<std::uint32_t>& directories,int directoryDepth,std::stop_token token){
    checkCancellation(token);if(offset==0&&!entries.empty()&&depth==0)return;
    if(depth>MaximumDirectoryDepth)throw MediaError(DiagnosticCode::XdvDepth);
    if(offset>table.size()||MinimumEntryLength>table.size()-offset||!visited.insert(offset).second){warnings.push_back({DiagnosticCode::XdvPointer,{}});return;}
    const auto left=Binary::little<std::uint16_t>(table,offset+LeftChildOffset),right=Binary::little<std::uint16_t>(table,offset+RightChildOffset);
    const auto start=Binary::little<std::uint32_t>(table,offset+FileStartSectorOffset),length=Binary::little<std::uint32_t>(table,offset+FileLengthOffset);
    const auto attributes=table[offset+FileAttributesOffset],nameLength=table[offset+FileNameLengthOffset];const auto entryLength=FileNameOffset+std::size_t(nameLength),padded=(entryLength+AlignmentMask)&~std::size_t(AlignmentMask);
    if(entryLength<MinimumEntryLength||padded>table.size()-offset)throw MediaError(DiagnosticCode::XdvEntryRange);
    auto name=Optical::ascii(table.subspan(offset+FileNameOffset,nameLength));while(!name.empty()&&name.back()==Null)name.pop_back();
    if(QString::fromStdString(name).trimmed().isEmpty()){auto digits=std::to_string(offset/PointerUnit);if(digits.size()<std::size_t(NameDigits))digits.insert(digits.begin(),std::size_t(NameDigits)-digits.size(),Zero);name=std::string(DefaultNamePrefix)+digits;}
    if(left)node(ctx,table,std::size_t(left)*PointerUnit,depth+1,visited,entries,warnings,directories,directoryDepth,token);
    const bool isDirectory=(attributes&DirectoryAttribute)!=0;
    FileSystemEntry entry;entry.name=std::move(name);entry.kind=isDirectory?EntryKind::Directory:EntryKind::File;entry.size=length;entry.rawAttributes=attributes;entry.storageReference=std::min(start,std::uint32_t(std::numeric_limits<int>::max()));
    if(isDirectory&&length)entry.children=directory(ctx,start,length,warnings,directories,directoryDepth+1,token);
    else if(!isDirectory&&length<=MaximumFileContentLength)entry.content=extent(*ctx.track,start,length,token);
    entry.nativeTypeId=isDirectory?DirectoryEntryType:FileEntryType;entry.occupiedSize=std::uint64_t(Optical::add(length,SectorSize-1)/SectorSize)*SectorSize;entry.attributes={std::string(FileAttribute)};entry.dataValid=isDirectory||entry.content.has_value();entry.metadata={{std::string(SectorMetadata),std::to_string(start)},{std::string(LengthMetadata),std::to_string(length)}};entries.push_back(std::move(entry));
    if(right)node(ctx,table,std::size_t(right)*PointerUnit,depth+1,visited,entries,warnings,directories,directoryDepth,token);
}
std::vector<FileSystemEntry> directory(const Context& ctx,std::uint32_t start,std::uint32_t length,std::vector<Diagnostic>& warnings,std::set<std::uint32_t>& directories,int depth,std::stop_token token){
    if(!length)return {};if(depth>MaximumDirectoryDepth)throw MediaError(DiagnosticCode::XdvDepth);
    Optical::ActiveExtent active(directories,start);if(!active)throw MediaError(DiagnosticCode::XdvDirectoryCycle);
    const auto table=extent(*ctx.track,start,length,token);std::set<std::size_t> visited;std::vector<FileSystemEntry> result;node(ctx,table,0,0,visited,result,warnings,directories,depth,token);return result;
}
}
std::string_view XdvdfsReader::id()const noexcept{return FileSystemIds::Xdvdfs;}
bool XdvdfsReader::canRead(const ImageDocument& d,const VolumeDescriptor& v,std::stop_token t)const{try{static_cast<void>(Xdv::context(d,v,t));return true;}catch(const MediaError& e){if(Optical::probeFailure(e))return false;throw;}}
FileSystemVolume XdvdfsReader::read(const ImageDocument& d,const VolumeDescriptor& v,std::stop_token t)const{const auto ctx=Xdv::context(d,v,t);FileSystemVolume result;result.name=XdvdfsConstants::FileSystemDisplayName;result.fileSystemDisplayName=XdvdfsConstants::FileSystemDisplayName;result.fileSystemId=id();result.capacity=std::uint64_t(ctx.track->sectorCount())*std::uint32_t(ctx.track->userDataLength());result.freeSpaceKnown=false;result.attributes={std::string(XdvdfsConstants::FileAttribute)};std::set<std::uint32_t> directories;result.entries=Xdv::directory(ctx,ctx.rootSector,ctx.rootSize,result.warnings,directories,0,t);return result;}
}
