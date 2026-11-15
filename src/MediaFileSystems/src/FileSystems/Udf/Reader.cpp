#include "Internal.h"
#include <algorithm>
#include <limits>
#include <map>
namespace RetroMediaLab::MediaFileSystems::Udf {
std::vector<std::uint8_t> physical(const OpticalTrack& track,std::uint32_t block,std::stop_token token){auto bytes=Optical::sector(track,block,token);if(bytes.size()!=LogicalBlockSize)throw MediaError(DiagnosticCode::UdfPhysicalSize,{std::to_string(bytes.size())});return bytes;}
std::vector<std::uint8_t> partitionBlock(const Context& ctx,std::uint32_t block,std::stop_token token){if(block>=ctx.length)throw MediaError(DiagnosticCode::UdfPartitionBlock);return physical(*ctx.track,Optical::add(ctx.start,block),token);}
std::pair<std::uint32_t,std::uint16_t> longAllocation(Bytes bytes,std::size_t offset){return {little<std::uint32_t>(bytes,offset+LongAllocationDescriptorBlockOffset),little<std::uint16_t>(bytes,offset+LongAllocationDescriptorPartitionOffset)};}
void primary(std::uint16_t partition){if(partition)throw MediaError(DiagnosticCode::UdfPartitionReference);}
namespace {
void recognition(const OpticalTrack& track,std::stop_token token){bool found=false;for(int index=0;index<MaximumVolumeRecognitionSectors;++index){const auto block=std::uint32_t(FirstVolumeRecognitionSector+index);if(std::int64_t(block)>=track.sectorCount())break;const auto bytes=physical(track,block,token);const auto id=Optical::ascii(Bytes(bytes).subspan(VolumeStructureIdentifierOffset,VolumeStructureIdentifierLength));if(id==Nsr02Identifier||id==Nsr03Identifier)found=true;if(id==TerminatingExtendedAreaIdentifier)break;}if(!found)throw MediaError(DiagnosticCode::UdfNsrMissing);}
std::uint32_t anchor(const OpticalTrack& track,std::stop_token token){
    std::vector<std::int64_t> candidates={AnchorSector};if(track.sectorCount()>0)candidates.push_back(track.sectorCount()-1);if(track.sectorCount()>AnchorSector)candidates.push_back(track.sectorCount()-AnchorSector-1);std::set<std::int64_t> visited;
    for(const auto candidate:candidates){if(candidate<0||candidate>=track.sectorCount()||candidate>std::numeric_limits<std::uint32_t>::max()||!visited.insert(candidate).second)continue;
        try{const auto block=std::uint32_t(candidate);const auto bytes=physical(track,block,token);validate(bytes,AnchorDescriptorTagId,block);return block;}
        catch(const MediaError& error){if(error.kind()!=ErrorKind::InvalidData)throw;}
    }
    throw MediaError(DiagnosticCode::UdfAnchorMissing);
}
}
Context context(const ImageDocument& document,const VolumeDescriptor& volume,std::stop_token token){
    const auto track=Optical::resolve(document,volume,DiagnosticCode::UdfTrackRequired,DiagnosticCode::UdfNoTrack);recognition(*track,token);const auto anchorLocation=anchor(*track,token);const auto bytes=physical(*track,anchorLocation,token);validate(bytes,AnchorDescriptorTagId,anchorLocation);
    const auto sequenceLength=little<std::uint32_t>(bytes,AnchorMainVolumeDescriptorSequenceExtentOffset+ExtentLengthOffset)&AllocationDescriptorLengthMask;
    const auto sequenceLocation=little<std::uint32_t>(bytes,AnchorMainVolumeDescriptorSequenceExtentOffset+ExtentLocationOffset);
    if(!sequenceLength||sequenceLength%LogicalBlockSize)throw MediaError(DiagnosticCode::UdfSequenceExtent);
    std::optional<std::vector<std::uint8_t>> logical;std::map<std::uint16_t,std::pair<std::uint32_t,std::uint32_t>> partitions;
    for(std::uint32_t index=0;index<sequenceLength/LogicalBlockSize;++index){
        const auto location=Optical::add(sequenceLocation,index);const auto descriptor=physical(*track,location,token);const auto tag=little<std::uint16_t>(descriptor,0);
        if(tag==TerminatingDescriptorTagId){validate(descriptor,tag,location);break;}
        if(tag==PartitionDescriptorTagId){validate(descriptor,tag,location);const auto number=little<std::uint16_t>(descriptor,PartitionNumberOffset);partitions[number]={little<std::uint32_t>(descriptor,PartitionStartingLocationOffset),little<std::uint32_t>(descriptor,PartitionLengthOffset)};}
        else if(tag==LogicalVolumeDescriptorTagId){validate(descriptor,tag,location);logical=descriptor;}
    }
    if(!logical)throw MediaError(DiagnosticCode::UdfLogicalMissing);const Bytes data=*logical;
    const auto blockSize=little<std::uint32_t>(data,LogicalVolumeBlockSizeOffset);if(blockSize!=LogicalBlockSize)throw MediaError(DiagnosticCode::UdfBlockSize,{std::to_string(blockSize)});
    const auto version=little<std::uint16_t>(data,LogicalVolumeDomainIdentifierOffset+EntityIdentifierSuffixOffset);if(std::find(SupportedRevisions.begin(),SupportedRevisions.end(),version)==SupportedRevisions.end())throw MediaError(DiagnosticCode::UdfRevision,{revision(version)});
    const auto mapLength=little<std::uint32_t>(data,LogicalVolumeMapTableLengthOffset),mapCount=little<std::uint32_t>(data,LogicalVolumePartitionMapCountOffset);
    if(mapCount!=1||mapLength!=TypeOnePartitionMapLength)throw MediaError(DiagnosticCode::UdfSinglePartition);
    const auto map=Optical::slice(data,LogicalVolumePartitionMapsOffset,TypeOnePartitionMapLength);if(map[0]!=TypeOnePartitionMap||map[1]!=TypeOnePartitionMapLength)throw MediaError(DiagnosticCode::UdfMapUnsupported);
    const auto number=little<std::uint16_t>(map,TypeOnePartitionMapPartitionNumberOffset);const auto partition=partitions.find(number);if(partition==partitions.end()||!partition->second.second)throw MediaError(DiagnosticCode::UdfPartitionMissing);
    const auto [fileSetBlock,fileSetReference]=longAllocation(data,LogicalVolumeContentsUseOffset);primary(fileSetReference);
    Context ctx{track,partition->second.first,partition->second.second,version,{},0};const auto fileSet=partitionBlock(ctx,fileSetBlock,token);validate(fileSet,FileSetDescriptorTagId,fileSetBlock);
    const auto [root,reference]=longAllocation(fileSet,FileSetRootDirectoryIcbOffset);primary(reference);ctx.root=root;ctx.name=dstring(Optical::slice(data,LogicalVolumeIdentifierOffset,LogicalVolumeIdentifierLength));return ctx;
}
Icb icb(const Context& ctx,std::uint32_t block,std::stop_token token){
    const auto bytes=partitionBlock(ctx,block,token);const auto tag=little<std::uint16_t>(bytes,0);if(tag!=FileEntryDescriptorTagId&&tag!=ExtendedFileEntryDescriptorTagId)throw MediaError(DiagnosticCode::UdfIcbFileEntry,{std::to_string(block)});validate(bytes,tag,block);
    const bool extended=tag==ExtendedFileEntryDescriptorTagId;
    const auto attributes=little<std::uint32_t>(bytes,extended?ExtendedFileEntryExtendedAttributesLengthOffset:FileEntryExtendedAttributesLengthOffset),length=little<std::uint32_t>(bytes,extended?ExtendedFileEntryAllocationDescriptorsLengthOffset:FileEntryAllocationDescriptorsLengthOffset);
    const auto base=extended?ExtendedFileEntryVariableDataOffset:FileEntryVariableDataOffset;const auto offset=Optical::integer(std::uint64_t(base)+std::uint64_t(Optical::integer(attributes)));
    if(length>std::uint32_t(std::numeric_limits<int>::max())||std::size_t(offset)>bytes.size()||length>bytes.size()-std::size_t(offset))throw MediaError(DiagnosticCode::UdfAllocationRange);
    const auto flags=little<std::uint16_t>(bytes,IcbTagOffset+IcbFlagsOffset);const auto information=little<std::uint64_t>(bytes,extended?ExtendedFileEntryInformationLengthOffset:FileEntryInformationLengthOffset);if(information>std::uint64_t(std::numeric_limits<std::int64_t>::max()))throw MediaError(DiagnosticCode::UdfSignedLength);
    const auto allocations=Bytes(bytes).subspan(std::size_t(offset),length);
    return {block,bytes[IcbTagOffset+IcbFileTypeOffset],information,std::uint16_t(flags&IcbAllocationDescriptorTypeMask),std::vector<std::uint8_t>(allocations.begin(),allocations.end()),timestamp(bytes,extended?ExtendedFileEntryAccessTimeOffset:FileEntryAccessTimeOffset),timestamp(bytes,extended?ExtendedFileEntryModificationTimeOffset:FileEntryModificationTimeOffset),extended?timestamp(bytes,ExtendedFileEntryCreationTimeOffset):std::nullopt};
}
}
