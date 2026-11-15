#include "Constants.h"
#include "../../../Primitives/Metadata.h"
#include "../../../Primitives/Binary.h"
#include <RetroMediaLab/MediaFileSystems/FamicomDisk.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <QString>

namespace RetroMediaLab::MediaFileSystems {
namespace {
using namespace FamicomDiskConstants;
void readSide(Bytes side,int sideNumber,std::vector<FileSystemEntry>& entries,std::vector<Diagnostic>& warnings) {
    const auto warn=[&](DiagnosticCode code){warnings.push_back({code,{std::to_string(sideNumber)}});};
    if(side.size()<DiskInfoLength||side.front()!=DiskInfoBlockCode){warn(DiagnosticCode::FdsDiskInfo);return;}
    std::size_t offset=DiskInfoLength;
    if(FileCountLength>side.size()-offset||side[offset]!=FileCountBlockCode){warn(DiagnosticCode::FdsFileCount);return;}
    const auto count=side[offset+1];offset+=FileCountLength;
    for(unsigned index=0;index<count;++index){
        if(FileHeaderLength>side.size()-offset||side[offset]!=FileHeaderBlockCode||!std::equal(FileIdentifier.begin(),FileIdentifier.end(),side.begin()+std::ptrdiff_t(offset+FileIdentifierOffset))){warn(DiagnosticCode::FdsFileHeader);return;}
        const auto number=side[offset+FileNumberOffset];
        const auto address=Binary::little<std::uint16_t>(side,offset+FileAddressOffset);
        const auto length=Binary::little<std::uint16_t>(side,offset+FileSizeOffset);
        const auto type=side[offset+FileTypeOffset];
        const auto header=offset+FileHeaderLength;
        if(header>=side.size()||side[header]!=FileDataBlockCode||length>side.size()-header-1){warnings.push_back({DiagnosticCode::FdsIncomplete,{std::to_string(number),std::to_string(sideNumber)}});return;}
        std::string name;
        for(const auto byte:side.subspan(offset+FileNameOffset,FileNameLength))name.push_back(byte>AsciiMaximum?AsciiReplacement:char(byte));
        while(!name.empty()&&(name.back()==Null||name.back()==Space))name.pop_back();
        const auto synthetic=QString::fromUtf8(name.data(),qsizetype(name.size())).trimmed().isEmpty();
        if(synthetic){auto digits=std::to_string(number);if(digits.size()<std::size_t(NameDigits))digits.insert(digits.begin(),std::size_t(NameDigits)-digits.size(),Zero);name=std::string(DefaultFileName)+digits;}
        const auto dataOffset=header+1;
        FileSystemEntry entry;
        entry.name=std::move(name);entry.kind=EntryKind::File;entry.size=length;entry.storageReference=sideNumber;
        entry.content=std::vector<std::uint8_t>(side.begin()+std::ptrdiff_t(dataOffset),side.begin()+std::ptrdiff_t(dataOffset+length));
        entry.nativeTypeId=FileEntryType;entry.occupiedSize=FileHeaderLength+std::uint64_t(1)+length;entry.attributes={std::string(SideAttribute)};entry.dataValid=true;entry.syntheticName=synthetic;
        entry.metadata={{std::string(SideMetadata),std::to_string(sideNumber)},{std::string(FileNumberMetadata),std::to_string(number)},{std::string(FileAddressMetadata),std::to_string(address)},{std::string(FileTypeMetadata),std::to_string(type)}};
        entries.push_back(std::move(entry));offset=dataOffset+length;
    }
}
}
std::string_view FamicomDiskReader::id() const noexcept {return FamicomDiskConstants::Id;}
bool FamicomDiskReader::canRead(const ImageDocument& document,const VolumeDescriptor& volume,std::stop_token token) const {
    checkCancellation(token);
    const auto& format=document.formatId();
    if(QString::fromUtf8(format.data(),qsizetype(format.size())).compare(QString::fromUtf8(Formats::NintendoFamicomDisk.data(),qsizetype(Formats::NintendoFamicomDisk.size())),Qt::CaseInsensitive)!=0)return false;
    const auto blocks=dynamic_cast<const BlockRepresentation*>(&document.representation());
    return blocks&&blocks->capacity()>0&&volume.start()==0&&volume.length()==std::uint64_t(blocks->capacity());
}
FileSystemVolume FamicomDiskReader::read(const ImageDocument& document,const VolumeDescriptor& descriptor,std::stop_token token) const {
    using namespace FamicomDiskConstants;
    if(!canRead(document,descriptor,token))throw MediaError(DiagnosticCode::FdsUnsupported);
    const auto& blocks=dynamic_cast<const BlockRepresentation&>(document.representation());
    int count{},length{};
    if(!Metadata::metadataInt(document.metadata(),std::string(SideCount),count)||count<1)throw MediaError(DiagnosticCode::FdsSideCount);
    if(!Metadata::metadataInt(document.metadata(),std::string(SideLength),length)||length<1)throw MediaError(DiagnosticCode::FdsSideLength);
    FileSystemVolume volume;volume.name=DisplayName;volume.fileSystemId=id();volume.fileSystemDisplayName=DisplayName;volume.capacity=std::uint64_t(blocks.capacity());volume.freeSpaceKnown=false;volume.attributes={document.formatId(),std::string(FileAttribute)};
    for(int side=0;side<count;++side){checkCancellation(token);std::vector<std::uint8_t> data(std::size_t(length),std::uint8_t{});blocks.readExactlyAsync(std::int64_t(side)*length,data,token).get();readSide(data,side,volume.entries,volume.warnings);}
    if(volume.entries.empty()&&!volume.warnings.empty())throw MediaError(DiagnosticCode::FdsNoFiles);
    return volume;
}
}
