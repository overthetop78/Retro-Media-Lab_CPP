#include "Constants.h"
#include "../../../Primitives/Binary.h"
#include <RetroMediaLab/MediaFileSystems/GameCubeFst.h>
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include <QString>
#include <limits>

namespace RetroMediaLab::MediaFileSystems {
namespace {
using namespace GameCubeFstConstants;
bool equal(std::string_view left,std::string_view right) {
    return QString::fromUtf8(left.data(),qsizetype(left.size())).compare(QString::fromUtf8(right.data(),qsizetype(right.size())),Qt::CaseInsensitive)==0;
}
std::uint64_t capacity(const OpticalTrack& track) {
    const auto count=std::uint64_t(track.sectorCount()),size=std::uint64_t(track.userDataLength());
    if(size&&count>std::numeric_limits<std::uint64_t>::max()/size)throw MediaError(DiagnosticCode::GameCubeOverflow);
    return count*size;
}
void range(const OpticalTrack& track,std::uint32_t offset,std::uint32_t length) {
    const auto bytes=capacity(track);
    if(offset>bytes||length>bytes-offset)throw MediaError(DiagnosticCode::GameCubeExtent);
}
std::vector<std::uint8_t> readBytes(const OpticalTrack& track,std::uint32_t offset,int length,std::stop_token token) {
    if(!length)return {};
    range(track,offset,std::uint32_t(length));
    std::vector<std::uint8_t> result(std::size_t(length),std::uint8_t{});
    std::int64_t sectorNumber=offset/SectorSize;
    std::size_t sectorOffset=offset%SectorSize,destination=0;
    while(destination<result.size()){
        checkCancellation(token);
        const auto sector=track.readUserDataAsync(sectorNumber,token).get();
        if(sectorOffset>=sector.size())throw MediaError(DiagnosticCode::GameCubeSectorData);
        const auto available=std::min(result.size()-destination,sector.size()-sectorOffset);
        std::copy_n(sector.begin()+std::ptrdiff_t(sectorOffset),available,result.begin()+std::ptrdiff_t(destination));
        destination+=available;++sectorNumber;sectorOffset=0;
    }
    return result;
}
int checkedInt(std::uint32_t value) {
    if(value>std::uint32_t(std::numeric_limits<int>::max()))throw MediaError(DiagnosticCode::GameCubeOverflow);
    return int(value);
}
struct Context {std::shared_ptr<const OpticalTrack> track;std::vector<std::uint8_t> table;int entryCount,stringTable;};
Context context(const ImageDocument& document,const VolumeDescriptor& volume,std::stop_token token) {
    checkCancellation(token);
    if(!equal(document.formatId(),Formats::OpticalIso))throw MediaError(DiagnosticCode::GameCubeNotIso);
    const auto extension=document.metadata().find(std::string(SourceExtensionKey));
    if(extension==document.metadata().end()||(!equal(extension->second,SourceExtension)&&!equal(extension->second,IsoExtension)))throw MediaError(DiagnosticCode::GameCubeNotImage);
    const auto optical=dynamic_cast<const OpticalRepresentation*>(&document.representation());
    if(!optical)throw MediaError(DiagnosticCode::GameCubeTrackRequired);
    const auto tracks=optical->tracks();
    if(!tracks)throw MediaError(DiagnosticCode::GameCubeTrackRequired);
    std::shared_ptr<const OpticalTrack> track;
    for(const auto& candidate:*tracks)if(candidate&&!candidate->isAudio()&&(!volume.sessionNumber()||candidate->sessionNumber()==*volume.sessionNumber())&&(!volume.trackNumber()||candidate->trackNumber()==*volume.trackNumber())){track=candidate;break;}
    if(!track)throw MediaError(DiagnosticCode::GameCubeNoTrack);
    if(track->userDataLength()!=SectorSize)throw MediaError(DiagnosticCode::GameCubeSectorSize);
    const auto header=readBytes(*track,0,DiscHeaderLength,token);
    if(Binary::big<std::uint32_t>(header,DiscMagicOffset)!=DiscMagic)throw MediaError(DiagnosticCode::GameCubeHeader);
    const auto offset=Binary::big<std::uint32_t>(header,FstOffsetField),size=Binary::big<std::uint32_t>(header,FstSizeField);
    if(offset<SectorSize||size<FstEntrySize)throw MediaError(DiagnosticCode::GameCubeDescriptor);
    range(*track,offset,size);
    auto table=readBytes(*track,offset,checkedInt(size),token);
    const auto root=Binary::big<std::uint32_t>(table,0);
    if(!(root&DirectoryFlag)||(root&NameOffsetMask))throw MediaError(DiagnosticCode::GameCubeRoot);
    const auto count=Binary::big<std::uint32_t>(table,LengthField);
    if(count<1||count>MaximumEntryCount||std::uint64_t(count)*FstEntrySize>table.size())throw MediaError(DiagnosticCode::GameCubeCount);
    return {std::move(track),std::move(table),int(count),int(count)*FstEntrySize};
}
std::string name(const Context& ctx,std::uint32_t offset) {
    const auto position=std::uint64_t(ctx.stringTable)+offset;
    if(position>std::uint64_t(std::numeric_limits<int>::max()))throw MediaError(DiagnosticCode::GameCubeOverflow);
    if(position>=ctx.table.size())throw MediaError(DiagnosticCode::GameCubeNameOffset);
    const auto start=ctx.table.begin()+std::ptrdiff_t(position),end=std::find(start,ctx.table.end(),std::uint8_t{});
    const auto decoded=QString::fromUtf8(reinterpret_cast<const char*>(&*start),qsizetype(end-start));
    if(!decoded.trimmed().isEmpty())return decoded.toUtf8().toStdString();
    auto digits=std::to_string(offset);
    if(digits.size()<std::size_t(NameDigits))digits.insert(digits.begin(),std::size_t(NameDigits)-digits.size(),Zero);
    return std::string(DefaultNamePrefix)+digits;
}
FileSystemEntry entry(std::string name,EntryKind kind,std::uint32_t length,int index) {
    FileSystemEntry result;result.name=std::move(name);result.kind=kind;result.size=length;result.storageReference=index;result.nativeTypeId=kind==EntryKind::Directory?DirectoryEntryType:FileEntryType;result.occupiedSize=length;result.attributes={std::string(FileAttribute)};result.dataValid=true;result.metadata={{std::string(OffsetMetadata),std::to_string(index)},{std::string(LengthMetadata),std::to_string(length)}};return result;
}
std::vector<FileSystemEntry> directory(const Context& ctx,int directoryIndex,int endIndex,int depth,std::vector<Diagnostic>& warnings,std::stop_token token) {
    if(depth>MaximumDirectoryDepth)throw MediaError(DiagnosticCode::GameCubeDepth);
    const auto next=checkedInt(Binary::big<std::uint32_t>(ctx.table,std::size_t(directoryIndex)*FstEntrySize+LengthField));
    if(next<=directoryIndex||next>ctx.entryCount||next>endIndex)throw MediaError(DiagnosticCode::GameCubeDirectoryRange);
    std::vector<FileSystemEntry> result;
    for(int index=directoryIndex+1;index<next;){
        checkCancellation(token);
        const auto offset=std::size_t(index)*FstEntrySize;
        const auto word=Binary::big<std::uint32_t>(ctx.table,offset),location=Binary::big<std::uint32_t>(ctx.table,offset+LocationField),length=Binary::big<std::uint32_t>(ctx.table,offset+LengthField);
        auto fileName=name(ctx,word&NameOffsetMask);
        if(word&DirectoryFlag){const auto childEnd=checkedInt(length);auto children=directory(ctx,index,next,depth+1,warnings,token);auto child=entry(std::move(fileName),EntryKind::Directory,0,index);child.children=std::move(children);result.push_back(std::move(child));index=childEnd;continue;}
        range(*ctx.track,location,length);
        auto child=entry(std::move(fileName),EntryKind::File,length,index);
        if(length<=MaximumFileContentLength)child.content=readBytes(*ctx.track,location,checkedInt(length),token);
        else {child.dataValid=false;warnings.push_back({DiagnosticCode::GameCubeContentLimit,{child.name}});}
        result.push_back(std::move(child));++index;
    }
    return result;
}
}
std::string_view GameCubeFstReader::id() const noexcept {return FileSystemIds::NintendoGameCubeFst;}
bool GameCubeFstReader::canRead(const ImageDocument& document,const VolumeDescriptor& volume,std::stop_token token) const {
    try{static_cast<void>(context(document,volume,token));return true;}
    catch(const MediaError& error){if(error.kind()==ErrorKind::InvalidData||error.kind()==ErrorKind::Unsupported||error.kind()==ErrorKind::ArgumentOutOfRange||error.kind()==ErrorKind::Overflow)return false;throw;}
}
FileSystemVolume GameCubeFstReader::read(const ImageDocument& document,const VolumeDescriptor& descriptor,std::stop_token token) const {
    const auto ctx=context(document,descriptor,token);
    FileSystemVolume volume;volume.name=GameCubeFstConstants::DisplayName;volume.fileSystemId=id();volume.fileSystemDisplayName=GameCubeFstConstants::DisplayName;volume.capacity=capacity(*ctx.track);volume.freeSpaceKnown=false;volume.attributes={std::string(GameCubeFstConstants::FileAttribute)};volume.entries=directory(ctx,0,ctx.entryCount,0,volume.warnings,token);return volume;
}
}
