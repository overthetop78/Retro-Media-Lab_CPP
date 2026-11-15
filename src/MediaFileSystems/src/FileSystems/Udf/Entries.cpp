#include "Internal.h"
#include <RetroMediaLab/MediaFileSystems/OpticalReaders.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Udf {
namespace {
std::vector<std::uint8_t> partitionBytes(const Context& ctx,std::uint32_t block,std::size_t length,std::stop_token token){std::vector<std::uint8_t> result(length);std::size_t completed=0;while(completed<length){const auto source=partitionBlock(ctx,block++,token);const auto count=std::min(source.size(),length-completed);std::copy_n(source.begin(),count,result.begin()+std::ptrdiff_t(completed));completed+=count;}return result;}
}
std::vector<std::uint8_t> content(const Context& ctx,const Icb& item,std::stop_token token){
    if(item.length>std::uint64_t(std::numeric_limits<int>::max()))throw MediaError(DiagnosticCode::UdfMemoryLength);
    std::vector<std::uint8_t> result(std::size_t(item.length));
    if(item.allocationType==EmbeddedAllocationDescriptors){if(result.size()>item.allocations.size())throw MediaError(DiagnosticCode::UdfEmbeddedRange);std::copy_n(item.allocations.begin(),result.size(),result.begin());return result;}
    if(item.allocationType==ExtendedAllocationDescriptors)throw MediaError(DiagnosticCode::UdfExtendedAllocation);
    const auto size=item.allocationType==ShortAllocationDescriptors?ShortAllocationDescriptorSize:LongAllocationDescriptorSize;
    if((item.allocationType!=ShortAllocationDescriptors&&item.allocationType!=LongAllocationDescriptors)||item.allocations.size()%std::size_t(size))throw MediaError(DiagnosticCode::UdfAllocationList);
    std::size_t completed=0;
    for(std::size_t offset=0;offset<item.allocations.size()&&completed<result.size();offset+=size){
        checkCancellation(token);const auto descriptor=Bytes(item.allocations).subspan(offset,size);const auto rawLength=little<std::uint32_t>(descriptor,0);const auto type=rawLength>>AllocationDescriptorTypeShift,length=rawLength&AllocationDescriptorLengthMask;const auto block=little<std::uint32_t>(descriptor,ShortAllocationDescriptorBlockOffset);
        if(size==LongAllocationDescriptorSize)primary(little<std::uint16_t>(descriptor,LongAllocationDescriptorPartitionOffset));
        const auto count=std::min(std::size_t(length),result.size()-completed);
        if(type==0){const auto bytes=partitionBytes(ctx,block,count,token);std::copy(bytes.begin(),bytes.end(),result.begin()+std::ptrdiff_t(completed));}
        else if(type==ContinuationExtentType)throw MediaError(DiagnosticCode::UdfContinuation);
        completed+=count;
    }
    if(completed!=result.size())throw MediaError(DiagnosticCode::UdfAllocationCoverage);return result;
}
std::vector<FileSystemEntry> directory(const Context& ctx,const Icb& item,std::set<std::uint32_t>& visited,std::stop_token token){
    Optical::ActiveExtent active(visited,item.block);if(!active)throw MediaError(DiagnosticCode::UdfDirectoryRecursive,{std::to_string(item.block)});
    const auto bytes=content(ctx,item,token);std::vector<FileSystemEntry> result;std::size_t offset=0;
    while(offset<=bytes.size()&&DescriptorTagSize<=bytes.size()-offset){
        checkCancellation(token);const auto remaining=Bytes(bytes).subspan(offset);const auto tag=little<std::uint16_t>(remaining,0);
        if(!tag){offset=(offset/LogicalBlockSize+1)*LogicalBlockSize;continue;}
        if(tag!=FileIdentifierDescriptorTagId)throw MediaError(DiagnosticCode::UdfDirectoryTag,{std::to_string(tag)});validate(remaining,FileIdentifierDescriptorTagId,{});
        const auto characteristics=Optical::slice(remaining,FileIdentifierCharacteristicsOffset,1)[0],nameLength=Optical::slice(remaining,FileIdentifierLengthOffset,1)[0];const auto implementation=little<std::uint16_t>(remaining,FileIdentifierImplementationUseLengthOffset);
        const auto length=(FileIdentifierImplementationUseOffset+std::size_t(implementation)+nameLength+AlignmentMask)&~std::size_t(AlignmentMask);if(length>remaining.size())throw MediaError(DiagnosticCode::UdfFileIdentifierRange);
        if(!(characteristics&(FileIdentifierDeletedFlag|FileIdentifierParentFlag))){
            const auto name=compressedName(Optical::slice(remaining,FileIdentifierImplementationUseOffset+std::size_t(implementation),nameLength));const auto [block,reference]=longAllocation(remaining,FileIdentifierIcbOffset);primary(reference);const auto child=icb(ctx,block,token);const bool isDirectory=child.type==DirectoryFileType||(characteristics&FileIdentifierDirectoryFlag)!=0;
            FileSystemEntry entry;entry.name=name;entry.kind=isDirectory?EntryKind::Directory:EntryKind::File;entry.size=child.length;entry.modified=child.modified;entry.created=child.created;entry.accessed=child.accessed;entry.rawAttributes=characteristics;entry.storageReference=std::min(child.block,std::uint32_t(std::numeric_limits<int>::max()));
            if(isDirectory)entry.children=directory(ctx,child,visited,token);else if(child.length<=std::uint64_t(std::numeric_limits<int>::max()))entry.content=content(ctx,child,token);
            entry.nativeTypeId=std::to_string(child.type);entry.occupiedSize=child.allocationType==EmbeddedAllocationDescriptors?child.allocations.size():child.length;entry.dataValid=true;result.push_back(std::move(entry));
        }
        offset+=length;
    }
    return result;
}
}
namespace RetroMediaLab::MediaFileSystems {
std::string_view UdfReader::id()const noexcept{return FileSystemIds::Udf;}
bool UdfReader::canRead(const ImageDocument& d,const VolumeDescriptor& v,std::stop_token t)const{try{static_cast<void>(Udf::context(d,v,t));return true;}catch(const MediaError& e){if(Optical::probeFailure(e))return false;throw;}}
FileSystemVolume UdfReader::read(const ImageDocument& d,const VolumeDescriptor& v,std::stop_token t)const{const auto ctx=Udf::context(d,v,t);const auto root=Udf::icb(ctx,ctx.root,t);if(root.type!=UdfConstants::DirectoryFileType)throw MediaError(DiagnosticCode::UdfRootDirectory);std::set<std::uint32_t> visited;FileSystemVolume result;result.name=ctx.name;result.fileSystemId=id();result.capacity=std::uint64_t(ctx.length)*UdfConstants::LogicalBlockSize;result.freeSpaceKnown=false;result.attributes={std::string(UdfConstants::AttributePrefix)+Udf::revision(ctx.revision)};result.entries=Udf::directory(ctx,root,visited,t);return result;}
}
