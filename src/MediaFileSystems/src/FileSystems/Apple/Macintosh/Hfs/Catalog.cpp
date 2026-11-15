#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::MacHfs::Internal {
using namespace Constants;
namespace {
void record(Bytes node,std::size_t start,std::size_t end,const SectorImage& image,std::uint16_t allocationStart,std::uint32_t allocationSize,std::vector<Record>& result,std::vector<Diagnostic>& warnings,std::stop_token token){
    const auto keyLength=node[start];if(keyLength<MinimumKeyLength||start+1+keyLength>end)return;const auto key=start+1;const auto parent=Binary::big<std::uint32_t>(node,key+ParentIdOffset);const auto nameLength=node[key+NameLengthOffset];
    if(nameLength>MaximumNameLength||key+NameOffset+nameLength>end)return;auto name=Macintosh::decodeName(node.subspan(key+NameOffset,nameLength));auto offset=start+1+keyLength;if(offset%RecordAlignment)++offset;if(offset>=end)return;
    Record value;value.parent=parent;value.name=std::move(name);
    if(node[offset]==DirectoryRecordType&&offset+MinimumDirectoryRecordLength<=end){value.directory=true;value.id=Binary::big<std::uint32_t>(node,offset+DirectoryIdOffset);value.modified=Macintosh::fromSeconds(Binary::big<std::uint32_t>(node,offset+DirectoryModifiedOffset));value.type=DirectoryDescription;value.valid=true;result.push_back(std::move(value));}
    else if(node[offset]==FileRecordType&&offset+MinimumFileRecordLength<=end){
        const auto dataLength=Binary::big<std::uint32_t>(node,offset+DataForkLengthOffset),resourceLength=Binary::big<std::uint32_t>(node,offset+ResourceForkLengthOffset);
        auto data=extents(image,node.subspan(offset+DataForkExtentsOffset,EmbeddedExtentsLength),allocationStart,allocationSize,dataLength,token);auto resources=extents(image,node.subspan(offset+ResourceForkExtentsOffset,EmbeddedExtentsLength),allocationStart,allocationSize,resourceLength,token);
        extentWarnings(value.name,Macintosh::ForkKind::Data,dataLength,data,warnings);extentWarnings(value.name,Macintosh::ForkKind::Resource,resourceLength,resources,warnings);
        value.type=Macintosh::fileType(node.subspan(offset+FinderTypeOffset,FinderTypeLength));value.defaultType=value.type.empty();if(value.defaultType)value.type=DefaultFileDescription;
        value.id=Binary::big<std::uint32_t>(node,offset+FileIdOffset);value.modified=Macintosh::fromSeconds(Binary::big<std::uint32_t>(node,offset+FileModifiedOffset));value.size=std::uint64_t(dataLength)+resourceLength;value.valid=data.valid&&resources.valid;value.data=std::move(data.content);value.resources=std::move(resources.content);result.push_back(std::move(value));
    }
}
}
std::vector<Record> catalog(Bytes bytes,const SectorImage& image,std::uint16_t start,std::uint32_t size,std::vector<Diagnostic>& warnings,std::stop_token token){
    checkCancellation(token);if(bytes.size()<MinimumCatalogLength)throw MediaError(DiagnosticCode::HfsCatalogTruncated,{std::to_string(bytes.size()),std::to_string(MinimumCatalogLength)});
    auto nodeSize=Binary::big<std::uint16_t>(bytes,NodeSizeOffset);if(nodeSize<MinimumNodeSize||nodeSize>MaximumNodeSize||bytes.size()<nodeSize)nodeSize=DefaultNodeSize;
    std::vector<Record> result;
    for(std::size_t offset=0;offset+nodeSize<=bytes.size();offset+=nodeSize){
        checkCancellation(token);const auto node=bytes.subspan(offset,nodeSize);if(node.size()<NodeDescriptorLength||node[NodeKindOffset]!=std::uint8_t(LeafNodeKind))continue;const auto count=Binary::big<std::uint16_t>(node,RecordCountOffset);
        if(count>MaximumRecordCount||NodeDescriptorLength+(std::size_t(count)+1)*RecordOffsetLength>node.size())continue;
        for(std::size_t index=0;index<count;++index){checkCancellation(token);const auto first=Binary::big<std::uint16_t>(node,node.size()-RecordOffsetLength*(index+1)),end=Binary::big<std::uint16_t>(node,node.size()-RecordOffsetLength*(index+RecordEndOffsetEntries));if(first<NodeDescriptorLength||end<=first||end>node.size())continue;record(node,first,end,image,start,size,result,warnings,token);}
    }
    if(result.empty())warnings.push_back({DiagnosticCode::HfsNoRecords,{}});return result;
}
}
