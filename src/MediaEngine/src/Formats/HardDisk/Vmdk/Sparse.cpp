#include "Sparse.h"
#include "Descriptor.h"
#include "../../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vmdk {
namespace {
std::int64_t sectorOffset(std::uint64_t sector){if(sector>Constants::MaximumSigned/Constants::SectorSize)throw MediaError(DiagnosticCode::VmdkGeometry);return std::int64_t(sector*Constants::SectorSize);}
}
std::shared_ptr<const MediaDocument> readSparse(const RecognitionContext& context,bool includeRanges,std::stop_token token){
    using namespace Constants;checkCancellation(token);if(context.length()<HeaderSize)throw MediaError(DiagnosticCode::VmdkSource);const auto header=context.readAsync(0,HeaderSize,token).get();
    const auto u32=[&](std::size_t offset){return qFromLittleEndian<std::uint32_t>(header.data()+offset);};const auto u64=[&](std::size_t offset){return qFromLittleEndian<std::uint64_t>(header.data()+offset);};
    const auto version=u32(VersionOffset);if(version!=SparseVersion)throw MediaError(DiagnosticCode::VmdkSparseVersion,{std::to_string(version)});const auto flags=u32(FlagsOffset);
    if(flags&(CompressedGrain|MarkerFlag))throw MediaError(DiagnosticCode::VmdkCompression);if(header[UncleanOffset])throw MediaError(DiagnosticCode::VmdkUnclean);
    const auto capacitySectors=u64(CapacityOffset),grainSectors=u64(GrainSectorsOffset),descriptorOffset=u64(DescriptorOffset),descriptorSize=u64(DescriptorSizeOffset),directorySector=u64(DirectoryOffset);const auto entries=u32(TableEntriesOffset);
    if(directorySector==StreamDirectory)throw MediaError(DiagnosticCode::VmdkCompression);
    if(!capacitySectors||capacitySectors>MaximumSigned/SectorSize||!grainSectors||grainSectors>MaximumBuffer/SectorSize||!entries||entries>MaximumBuffer/EntryBytes||!directorySector)throw MediaError(DiagnosticCode::VmdkGeometry);
    if(descriptorOffset&&descriptorSize){if(descriptorSize>MaximumDescriptor/SectorSize)throw MediaError(DiagnosticCode::VmdkDescriptorSize);const auto offset=sectorOffset(descriptorOffset),length=std::int64_t(descriptorSize*SectorSize);if(offset>context.length()||length>context.length()-offset)throw MediaError(DiagnosticCode::VmdkDescriptorRange);const auto bytes=context.readAsync(offset,std::int32_t(length),token).get();auto text=QString::fromUtf8(reinterpret_cast<const char*>(bytes.data()),qsizetype(bytes.size()));while(text.endsWith(QChar(Null)))text.chop(1);validateDescriptor(text,MonolithicSparse);}
    const auto capacity=std::int64_t(capacitySectors*SectorSize),grainSize=std::int64_t(grainSectors*SectorSize);const auto grains=std::uint64_t(capacity)/grainSize+(capacity%grainSize!=0),directoryEntries=grains/entries+(grains%entries!=0);
    if(directoryEntries>MaximumBuffer/EntryBytes)throw MediaError(DiagnosticCode::VmdkDirectory);const auto directoryOffset=sectorOffset(directorySector);const auto directoryBytes=directoryEntries*EntryBytes;
    if(std::uint64_t(directoryOffset)>std::uint64_t(context.length())||directoryBytes>std::uint64_t(context.length()-directoryOffset))throw MediaError(DiagnosticCode::VmdkDirectory);
    const auto directory=context.readAsync(directoryOffset,std::int32_t(directoryBytes),token).get();std::shared_ptr<const MediaFileSystems::RandomAccessData> source;
    if(includeRanges){const auto& name=context.source()->primaryPath();source=fileSource(std::filesystem::path(std::u8string(name.begin(),name.end())));}std::vector<DataRange> ranges;std::uint64_t grainIndex=0;
    for(std::uint64_t index=0;index<directoryEntries;++index){checkCancellation(token);const auto tableSector=qFromLittleEndian<std::uint32_t>(directory.data()+std::size_t(index)*EntryBytes);if(!tableSector)throw MediaError(DiagnosticCode::VmdkTableMissing,{std::to_string(index)});const auto tableOffset=std::int64_t(tableSector)*SectorSize;const auto tableLength=std::uint64_t(entries)*EntryBytes;
        if(std::uint64_t(tableOffset)>std::uint64_t(context.length())||tableLength>std::uint64_t(context.length()-tableOffset))throw MediaError(DiagnosticCode::VmdkDirectory);const auto table=context.readAsync(tableOffset,std::int32_t(tableLength),token).get();
        for(std::uint32_t entry=0;entry<entries&&grainIndex<grains;++entry,++grainIndex){checkCancellation(token);const auto sector=qFromLittleEndian<std::uint32_t>(table.data()+std::size_t(entry)*EntryBytes);const auto address=std::int64_t(grainIndex*grainSize),length=std::min(grainSize,capacity-address);
            if(!sector){addBlockRange(ranges,address,length,DataRangeKind::Unallocated);continue;}if(sector==1&&(flags&ZeroedGrain)){addBlockRange(ranges,address,length,DataRangeKind::Zero);continue;}const auto offset=std::int64_t(sector)*SectorSize;if(offset>context.length()||length>context.length()-offset)throw MediaError(DiagnosticCode::VmdkGrainOutside,{std::to_string(grainIndex)});addBlockRange(ranges,address,length,includeRanges?DataRangeKind::Stored:DataRangeKind::Unavailable,source,includeRanges?offset:0);
        }
    }
    return createDocument(context.source(),capacity,std::move(ranges),false);
}
}
