#include "Structures.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vhdx {
Metadata readMetadata(const RecognitionContext& context,Region region,std::stop_token token){
    using namespace Constants;if(region.length<MetadataTableSize)throw MediaError(DiagnosticCode::VhdxMetadataRange,{guidText(MetadataGuid)});const auto header=context.readAsync(region.offset,MetadataTableSize,token).get();if(!std::equal(MetadataSignature.begin(),MetadataSignature.end(),header.begin()))throw MediaError(DiagnosticCode::VhdxMetadataSignature);const auto count=qFromLittleEndian<std::uint16_t>(header.data()+MetadataCountOffset);if(count>MaximumEntries||MetadataEntriesOffset+count*StructureEntrySize>header.size())throw MediaError(DiagnosticCode::VhdxMetadataCount);
    std::optional<std::uint32_t> blockSize,fileFlags,logical,physical;std::optional<std::uint64_t> virtualSize;std::optional<Guid> page83;
    for(std::uint16_t index=0;index<count;++index){checkCancellation(token);const auto entry=std::span(header).subspan(MetadataEntriesOffset+std::size_t(index)*StructureEntrySize,StructureEntrySize);const auto id=readGuid(entry);const auto offset=qFromLittleEndian<std::uint32_t>(entry.data()+MetadataItemOffset),length=qFromLittleEndian<std::uint32_t>(entry.data()+MetadataItemLengthOffset),flags=qFromLittleEndian<std::uint32_t>(entry.data()+MetadataItemFlagsOffset);
        if(offset>region.length||length>region.length-offset||length>MaximumBuffer)throw MediaError(DiagnosticCode::VhdxMetadataRange,{guidText(id)});
        if(std::find(MetadataIds.begin(),MetadataIds.end(),id)==MetadataIds.end()){if(flags&MetadataRequired)throw MediaError(DiagnosticCode::VhdxRequiredMetadata,{guidText(id)});continue;}
        const auto item=context.readAsync(region.offset+offset,std::int32_t(length),token).get();
        if(id==FileParametersGuid&&length>=ParametersSize){blockSize=qFromLittleEndian<std::uint32_t>(item.data());fileFlags=qFromLittleEndian<std::uint32_t>(item.data()+ParametersFlagsOffset);}
        else if(id==DiskSizeGuid&&length==DiskSizeBytes)virtualSize=qFromLittleEndian<std::uint64_t>(item.data());
        else if(id==LogicalSectorGuid&&length==SectorSizeBytes)logical=qFromLittleEndian<std::uint32_t>(item.data());
        else if(id==PhysicalSectorGuid&&length==SectorSizeBytes)physical=qFromLittleEndian<std::uint32_t>(item.data());
        else if(id==Page83Guid&&length==GuidBytes)page83=readGuid(item);
    }
    if(!blockSize||!fileFlags||!logical||!physical||!virtualSize||!page83)throw MediaError(DiagnosticCode::VhdxMissingMetadata);
    if(*blockSize<MiB||*blockSize>MaximumBlockSize||(*blockSize&(*blockSize-1)))throw MediaError(DiagnosticCode::VhdxBlockSize);
    if((*logical!=LegacySector&&*logical!=LargeSector)||(*physical!=LegacySector&&*physical!=LargeSector))throw MediaError(DiagnosticCode::VhdxSectorSize);
    if(!*virtualSize||*virtualSize>MaximumSigned||*virtualSize%*logical)throw MediaError(DiagnosticCode::VhdxVirtualSize);
    return {*blockSize,*fileFlags,std::int64_t(*virtualSize),std::int32_t(*logical),std::int32_t(*physical),*page83};
}
}
