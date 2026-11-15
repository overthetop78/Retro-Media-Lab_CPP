#include "Internal.h"
#include "../../Primitives/Binary.h"
#include <RetroMediaLab/MediaFileSystems/OpticalReaders.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Iso {
using namespace IsoConstants;
std::pair<std::uint32_t,std::uint32_t> root(Bytes source){const auto offset=std::size_t(RootDirectoryRecordOffset);if(offset>=source.size())throw MediaError(DiagnosticCode::OpticalBounds);const auto length=source[offset];if(length<DirectoryRecordMinimumLength||length>source.size()-offset)throw MediaError(DiagnosticCode::IsoRootRecord);return {Binary::little<std::uint32_t>(source,offset+DirectoryExtentOffset),Binary::little<std::uint32_t>(source,offset+DirectoryDataLengthOffset)};}
std::vector<std::uint8_t> extent(const OpticalTrack& track,std::uint32_t block,std::uint32_t length,std::stop_token token){
    if(length>std::uint32_t(std::numeric_limits<int>::max()))throw MediaError(DiagnosticCode::IsoExtentLarge);
    std::vector<std::uint8_t> result(length);std::size_t completed=0;
    while(completed<result.size()){
        const auto bytes=Optical::sector(track,block++,token);if(bytes.empty())throw MediaError(DiagnosticCode::OpticalEmptySector);
        const auto count=std::min(bytes.size(),result.size()-completed);std::copy_n(bytes.begin(),count,result.begin()+std::ptrdiff_t(completed));completed+=count;
    }
    return result;
}
std::vector<std::uint8_t> descriptor(const ImageDocument& document,const VolumeDescriptor& volume,Mode mode,std::stop_token token){
    const auto track=Optical::resolve(document,volume,DiagnosticCode::IsoTrackRequired,DiagnosticCode::IsoNoTrack);
    for(std::int64_t block=FirstVolumeDescriptorBlock;;++block){
        const auto bytes=Optical::sector(*track,block,token);if(bytes.size()<LogicalBlockSize)throw MediaError(DiagnosticCode::IsoDescriptorIncomplete);
        if(Optical::ascii(Bytes(bytes).subspan(VolumeDescriptorIdentifierOffset,VolumeDescriptorIdentifierLength))!=VolumeDescriptorIdentifier)throw MediaError(DiagnosticCode::IsoDescriptorIdentifier);
        if(acceptDescriptor(bytes,mode)&&acceptFileSystem(*track,bytes,mode,token))return bytes;
        if(bytes[0]==VolumeDescriptorTerminatorType)throw MediaError(DiagnosticCode::IsoDescriptorAbsent);
    }
}
namespace {
template<class T> T both(Bytes data,std::size_t offset,DiagnosticCode code){const auto little=Binary::little<T>(data,offset),big=Binary::big<T>(data,offset+sizeof(T));if(little!=big)throw MediaError(code);return little;}
std::optional<Timestamp> date(Bytes bytes){
    if(bytes.size()<RecordingDateLength||!bytes[0])return {};
    Timestamp value;value.year=bytes[0]+RecordingDateYear;value.month=bytes[RecordingMonthOffset];value.day=bytes[RecordingDayOffset];value.hour=bytes[RecordingHourOffset];value.minute=bytes[RecordingMinuteOffset];value.second=bytes[RecordingSecondOffset];value.offsetMinutes=std::bit_cast<std::int8_t>(bytes[RecordingDateTimezoneIndex])*RecordingDateTimezoneMinutes;
    return validTimestamp(value)?std::optional(value):std::nullopt;
}
std::vector<FileSystemEntry> directory(const OpticalTrack& track,std::uint32_t block,std::uint32_t length,std::set<std::uint32_t>& visited,std::vector<Diagnostic>& warnings,Mode mode,std::stop_token token){
    Optical::ActiveExtent active(visited,block);if(!active){warnings.push_back({DiagnosticCode::IsoRecursive,{std::to_string(block)}});return {};}
    const auto data=extent(track,block,length,token);std::vector<FileSystemEntry> entries;std::size_t offset=0;
    while(offset<data.size()){
        checkCancellation(token);const auto count=data[offset];if(!count){offset=(offset/LogicalBlockSize+1)*LogicalBlockSize;continue;}
        if(count<DirectoryRecordMinimumLength||count>data.size()-offset)throw MediaError(DiagnosticCode::IsoDirectoryRecord);
        const auto record=Bytes(data).subspan(offset,count);const auto nameLength=record[DirectoryFileIdentifierLengthOffset];
        if(DirectoryFileIdentifierOffset+std::size_t(nameLength)>record.size())throw MediaError(DiagnosticCode::IsoFileIdentifier);
        const auto identifier=record.subspan(DirectoryFileIdentifierOffset,nameLength);
        if(!(nameLength==1&&(identifier[0]==0||identifier[0]==1))){
            FileSystemEntry entry;entry.name=entryName(record,fileName(identifier,mode),mode);
            const auto childExtent=Binary::little<std::uint32_t>(record,DirectoryExtentOffset),childLength=Binary::little<std::uint32_t>(record,DirectoryDataLengthOffset);
            const auto isDirectory=(record[DirectoryFlagsOffset]&DirectoryFlag)!=0;
            entry.kind=isDirectory?EntryKind::Directory:EntryKind::File;entry.size=childLength;entry.rawAttributes=record[DirectoryFlagsOffset];entry.storageReference=std::min(childExtent,std::uint32_t(std::numeric_limits<int>::max()));entry.modified=date(record.subspan(DirectoryDateOffset,RecordingDateLength));
            if(isDirectory)entry.children=directory(track,childExtent,childLength,visited,warnings,mode,token);
            else if(childLength<=std::uint32_t(std::numeric_limits<int>::max()))entry.content=extent(track,childExtent,childLength,token);
            const auto rounded=Optical::add(childLength,LogicalBlockSize-1)/LogicalBlockSize;entry.occupiedSize=std::uint64_t(rounded)*LogicalBlockSize;
            entries.push_back(std::move(entry));
        }
        offset+=count;
    }
    return entries;
}
}
FileSystemVolume read(const ImageDocument& document,const VolumeDescriptor& volume,Mode mode,std::string_view id,std::stop_token token){
    const auto bytes=descriptor(document,volume,mode,token);const auto blockSize=both<std::uint16_t>(bytes,LogicalBlockSizeOffset,DiagnosticCode::IsoEndian16);
    if(blockSize!=LogicalBlockSize)throw MediaError(DiagnosticCode::IsoBlockSize,{std::to_string(blockSize)});
    const auto volumeBlocks=both<std::uint32_t>(bytes,VolumeSpaceSizeOffset,DiagnosticCode::IsoEndian32);const auto [block,length]=root(bytes);
    const auto track=Optical::resolve(document,volume,DiagnosticCode::IsoTrackRequired,DiagnosticCode::IsoNoTrack);std::set<std::uint32_t> visited;
    FileSystemVolume result;result.fileSystemId=id;result.capacity=std::uint64_t(volumeBlocks)*blockSize;result.entries=directory(*track,block,length,visited,result.warnings,mode,token);result.name=volumeName(Bytes(bytes).subspan(VolumeIdentifierOffset,VolumeIdentifierLength),mode);result.freeSpaceKnown=false;result.attributes={std::string(Attribute)};return result;
}
}
namespace RetroMediaLab::MediaFileSystems {
namespace {
bool canReadIso(const ImageDocument& document,const VolumeDescriptor& volume,Iso::Mode mode,std::stop_token token){try{static_cast<void>(Iso::descriptor(document,volume,mode,token));return true;}catch(const MediaError& error){if(Optical::probeFailure(error,false))return false;throw;}}
}
std::string_view Iso9660Reader::id()const noexcept{return FileSystemIds::Iso9660;}
bool Iso9660Reader::canRead(const ImageDocument& d,const VolumeDescriptor& v,std::stop_token t)const{return canReadIso(d,v,Iso::Mode::Primary,t);}
FileSystemVolume Iso9660Reader::read(const ImageDocument& d,const VolumeDescriptor& v,std::stop_token t)const{return Iso::read(d,v,Iso::Mode::Primary,id(),t);}
std::string_view JolietReader::id()const noexcept{return FileSystemIds::Joliet;}
bool JolietReader::canRead(const ImageDocument& d,const VolumeDescriptor& v,std::stop_token t)const{return canReadIso(d,v,Iso::Mode::Joliet,t);}
FileSystemVolume JolietReader::read(const ImageDocument& d,const VolumeDescriptor& v,std::stop_token t)const{return Iso::read(d,v,Iso::Mode::Joliet,id(),t);}
std::string_view RockRidgeReader::id()const noexcept{return FileSystemIds::RockRidge;}
bool RockRidgeReader::canRead(const ImageDocument& d,const VolumeDescriptor& v,std::stop_token t)const{return canReadIso(d,v,Iso::Mode::RockRidge,t);}
FileSystemVolume RockRidgeReader::read(const ImageDocument& d,const VolumeDescriptor& v,std::stop_token t)const{return Iso::read(d,v,Iso::Mode::RockRidge,id(),t);}
}
