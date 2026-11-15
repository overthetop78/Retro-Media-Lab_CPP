#include "Fat12Internal.h"
#include "../../../Primitives/Binary.h"
#include "../../../Primitives/MetadataConstants.h"
#include <QString>
namespace RetroMediaLab::MediaFileSystems::Fat12::Internal {
using namespace FatConstants::FatDirectoryLayout;
using namespace Fat12Constants;
namespace {
std::string combine(const std::string& path,const std::string& name) {return path.empty()?name:path+MetadataConstants::PathSeparator+name;}
QString ordinalUpper(const std::string& name) {
    auto scalars=QString::fromUtf8(name).toStdU32String();
    for(auto& scalar:scalars){const auto upper=QChar::toUpper(scalar);if(scalar<MetadataConstants::AsciiLimit || upper>=MetadataConstants::AsciiLimit)scalar=upper;}
    return QString::fromUcs4(scalars.data(),qsizetype(scalars.size()));
}
FileSystemEntry readEntry(ReadContext& context,const Range& directoryRange,const Range& fat,const Layout& layout,std::uint32_t depth,const std::string& path,std::size_t offset,std::string name,std::uint8_t attributes) {
    const Bytes bytes=directoryRange.bytes;
    const auto cluster=Binary::little<std::uint16_t>(bytes,offset+FirstClusterOffset);
    const auto size=Binary::little<std::uint32_t>(bytes,offset+FileSizeOffset);
    const bool isDirectory=(attributes&DirectoryAttribute)!=0;
    auto chain=clusterChain(context,fat,layout,cluster,name);
    FileSystemEntry result;result.name=std::move(name);result.kind=isDirectory?EntryKind::Directory:EntryKind::File;result.size=size;
    result.rawAttributes=attributes;result.storageReference=cluster;result.metadataValid=directoryRange.valid() && chain.valid;
    if(!directoryRange.trusted || !chain.trusted)result.dataValid=false;
    if(isDirectory) {
        const auto fullPath=combine(path,result.name);
        bool cycle=false;
        for(const auto used:chain.clusters)if(context.expandedDirectories.contains(used)){cycle=true;break;}
        if(cycle) {context.warnings.push_back({DiagnosticCode::FatDirectoryCycle,{fullPath,std::to_string(cluster)}});result.metadataValid=false;}
        else {
            context.expandedDirectories.insert(chain.clusters.begin(),chain.clusters.end());
            Range contents;contents.bytes=std::move(chain.bytes);contents.present.assign(std::max(std::size_t(1),contents.bytes.size()/context.image.geometry().blockSize),chain.valid);contents.trusted=chain.trusted;
            result.children=directory(context,contents,fat,layout,depth+1,fullPath);
        }
    } else {
        const auto available=std::min(std::uint64_t(chain.bytes.size()),std::uint64_t(size));
        if(size>chain.bytes.size()) {context.warnings.push_back({DiagnosticCode::FatIncompleteContent,{result.name,std::to_string(size),std::to_string(chain.bytes.size())}});result.metadataValid=false;}
        chain.bytes.resize(std::size_t(available));result.content=std::move(chain.bytes);
    }
    result.modified=Fat::decodeDateTime(Binary::little<std::uint16_t>(bytes,offset+ModifiedDateOffset),Binary::little<std::uint16_t>(bytes,offset+ModifiedTimeOffset));
    return result;
}
}
std::vector<FileSystemEntry> directory(ReadContext& context,const Range& directoryRange,const Range& fat,const Layout& layout,std::uint32_t depth,const std::string& path) {
    if(depth>Fat12DirectoryReader::MaximumDepth){context.warnings.push_back({DiagnosticCode::FatDepthLimit,{path,std::to_string(depth)}});return {};}
    std::vector<FileSystemEntry> entries;const auto& bytes=directoryRange.bytes;
    for(std::size_t offset=0;offset+EntrySize<=bytes.size();offset+=EntrySize) {
        checkCancellation(context.token);const auto first=bytes[offset];if(first==EndMarker)break;if(first==DeletedMarker)continue;
        const auto attributes=bytes[offset+AttributesOffset];if((attributes&LongFileName)==LongFileName || (attributes&VolumeLabelAttribute)!=0)continue;
        const auto name=Fat::decodeName(Bytes(bytes).subspan(offset,NameLength+ExtensionLength));
        if(name.empty() || name==CurrentDirectoryName || name==ParentDirectoryName)continue;
        entries.push_back(readEntry(context,directoryRange,fat,layout,depth,path,offset,name,attributes));
    }
    std::stable_sort(entries.begin(),entries.end(),[](const auto& a,const auto& b){
        const bool ad=a.kind==EntryKind::Directory,bd=b.kind==EntryKind::Directory;
        if(ad!=bd)return ad;
        return ordinalUpper(a.name).compare(ordinalUpper(b.name),Qt::CaseSensitive)<0;
    });
    return entries;
}
}
