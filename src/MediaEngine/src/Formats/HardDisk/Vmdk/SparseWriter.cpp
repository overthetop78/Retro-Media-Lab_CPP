#include "Writing.h"
#include "Descriptor.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vmdk {
void writeSparse(const BlockImage& blocks,const std::filesystem::path& name,const SeekableImageOutput& output,std::stop_token token){
    using namespace Constants;const auto grainSize=std::uint64_t(DefaultGrainSectors)*SectorSize,capacity=std::uint64_t(blocks.capacity());const auto grains=capacity/grainSize+(capacity%grainSize!=0),tableCount=grains/DefaultTableEntries+(grains%DefaultTableEntries!=0);
    if(tableCount>MaximumBuffer/EntryBytes)throw MediaError(DiagnosticCode::VmdkWriteLimits);const auto directorySector=DescriptorSector+DescriptorSectors,directorySectors=(tableCount*EntryBytes+SectorSize-1)/SectorSize,tableSectors=(std::uint64_t(DefaultTableEntries)*EntryBytes+SectorSize-1)/SectorSize,tablesSector=directorySector+directorySectors,overhead=tablesSector+tableCount*tableSectors;
    const auto tablesBytes=tableCount*tableSectors*SectorSize;if(tablesBytes>MaximumBuffer||directorySectors*SectorSize>MaximumBuffer||tablesSector+(tableCount-1)*tableSectors>MaximumField)throw MediaError(DiagnosticCode::VmdkWriteLimits);
    std::vector<std::uint8_t> header(HeaderSize);const auto u32=[&](std::size_t offset,std::uint32_t value){qToLittleEndian(value,header.data()+offset);};const auto u64=[&](std::size_t offset,std::uint64_t value){qToLittleEndian(value,header.data()+offset);};
    u32(0,SparseMagic);u32(VersionOffset,SparseVersion);u32(FlagsOffset,ValidNewline|ZeroedGrain);u64(CapacityOffset,capacity/SectorSize);u64(GrainSectorsOffset,DefaultGrainSectors);u64(DescriptorOffset,DescriptorSector);u64(DescriptorSizeOffset,DescriptorSectors);u32(TableEntriesOffset,DefaultTableEntries);u64(DirectoryOffset,directorySector);u64(OverheadOffset,overhead);header[NewlineOffset]=LineFeed;header[SpaceOffset]=Space;header[CarriageOffset]=CarriageReturn;header[LastNewlineOffset]=LineFeed;
    output.write(header);const auto descriptor=createDescriptor(blocks,MonolithicSparse,Sparse,name,{});std::vector<std::uint8_t> descriptorArea(static_cast<std::size_t>(DescriptorSectors*SectorSize));if(descriptor.size()>descriptorArea.size())throw MediaError(DiagnosticCode::VmdkGeneratedDescriptor);std::copy(descriptor.begin(),descriptor.end(),descriptorArea.begin());output.write(descriptorArea);
    std::vector<std::uint8_t> directory(static_cast<std::size_t>(directorySectors*SectorSize)),tables(static_cast<std::size_t>(tablesBytes));
    for(std::uint64_t index=0;index<tableCount;++index){checkCancellation(token);qToLittleEndian(std::uint32_t(tablesSector+index*tableSectors),directory.data()+std::size_t(index)*EntryBytes);}output.write(directory);const auto tablesOffset=output.position();output.write(tables);std::vector<std::uint8_t> grain(static_cast<std::size_t>(grainSize));
    for(std::uint64_t index=0;index<grains;++index){checkCancellation(token);std::fill(grain.begin(),grain.end(),0);const auto address=std::int64_t(index*grainSize),length=std::min(std::int64_t(grainSize),blocks.capacity()-address);blocks.readExactly(address,std::span(grain).first(std::size_t(length)),token);const auto entryOffset=(index/DefaultTableEntries)*tableSectors*SectorSize+(index%DefaultTableEntries)*EntryBytes;
        if(std::all_of(grain.begin(),grain.begin()+std::size_t(length),[](auto value){return value==0;})){qToLittleEndian(std::uint32_t(1),tables.data()+std::size_t(entryOffset));continue;}
        if(output.position()%SectorSize)throw MediaError(DiagnosticCode::VmdkWriteAlignment);const auto sector=output.position()/SectorSize;if(sector>MaximumField)throw MediaError(DiagnosticCode::VmdkWriteLimits);qToLittleEndian(std::uint32_t(sector),tables.data()+std::size_t(entryOffset));output.write(grain);
    }
    const auto end=output.position();output.seek(tablesOffset);output.write(tables);output.seek(end);
}
}
