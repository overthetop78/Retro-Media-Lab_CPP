#include "Writing.h"
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vhdx {
std::vector<std::uint8_t> createIdentifier(){using namespace Constants;std::vector<std::uint8_t> bytes(IdentifierSize);std::copy(FileSignature.begin(),FileSignature.end(),bytes.begin());for(std::size_t index=0;index<Creator.size();++index)qToLittleEndian(std::uint16_t(std::uint8_t(Creator[index])),bytes.data()+CreatorOffset+index*Utf16Bytes);return bytes;}
std::vector<std::uint8_t> createHeader(std::uint64_t sequence,const Guid& file,const Guid& data,std::uint64_t logOffset){
    using namespace Constants;std::vector<std::uint8_t> bytes(HeaderSize);std::copy(HeaderSignature.begin(),HeaderSignature.end(),bytes.begin());qToLittleEndian(sequence,bytes.data()+SequenceOffset);std::copy(file.begin(),file.end(),bytes.begin()+FileWriteIdOffset);std::copy(data.begin(),data.end(),bytes.begin()+DataWriteIdOffset);qToLittleEndian(Version,bytes.data()+HeaderVersionOffset);qToLittleEndian(MiB,bytes.data()+LogLengthOffset);qToLittleEndian(logOffset,bytes.data()+LogOffset);writeCrc(bytes);return bytes;
}
std::vector<std::uint8_t> createRegions(std::uint32_t batLength){
    using namespace Constants;std::vector<std::uint8_t> bytes(RegionTableSize);std::copy(RegionSignature.begin(),RegionSignature.end(),bytes.begin());qToLittleEndian(std::uint32_t(RegionEntryCount),bytes.data()+RegionCountOffset);
    const auto put=[&](std::size_t offset,const Guid& id,std::uint64_t address,std::uint32_t length){std::copy(id.begin(),id.end(),bytes.begin()+offset);qToLittleEndian(address,bytes.data()+offset+RegionFileOffset);qToLittleEndian(length,bytes.data()+offset+RegionLengthOffset);qToLittleEndian(RegionRequired,bytes.data()+offset+RegionFlagsOffset);};
    put(RegionEntriesOffset,BatGuid,BatRegionOffset,batLength);put(RegionEntriesOffset+StructureEntrySize,MetadataGuid,MetadataRegionOffset,MiB);writeCrc(bytes);return bytes;
}
std::vector<std::uint8_t> createMetadata(const BlockImage& blocks,std::uint32_t blockSize,bool fixed){
    using namespace Constants;std::vector<std::uint8_t> bytes(MiB);std::copy(MetadataSignature.begin(),MetadataSignature.end(),bytes.begin());qToLittleEndian(std::uint16_t(MetadataEntryCount),bytes.data()+MetadataCountOffset);auto cursor=MetadataValueOffset;
    for(std::size_t index=0;index<MetadataIds.size();++index){const auto offset=MetadataEntriesOffset+index*StructureEntrySize;std::copy(MetadataIds[index].begin(),MetadataIds[index].end(),bytes.begin()+offset);qToLittleEndian(std::uint32_t(cursor),bytes.data()+offset+MetadataItemOffset);qToLittleEndian(std::uint32_t(MetadataValueLengths[index]),bytes.data()+offset+MetadataItemLengthOffset);qToLittleEndian(MetadataValueFlags[index],bytes.data()+offset+MetadataItemFlagsOffset);
        const auto& id=MetadataIds[index];
        if(id==FileParametersGuid){qToLittleEndian(blockSize,bytes.data()+cursor);qToLittleEndian(fixed?LeaveAllocated:0,bytes.data()+cursor+ParametersFlagsOffset);}
        else if(id==DiskSizeGuid)qToLittleEndian(std::uint64_t(blocks.capacity()),bytes.data()+cursor);
        else if(id==Page83Guid){const auto page=randomGuid();std::copy(page.begin(),page.end(),bytes.begin()+cursor);}
        else qToLittleEndian(std::uint32_t(blocks.logicalBlockSize()),bytes.data()+cursor);
        cursor+=MetadataValueLengths[index];
    }return bytes;
}
}
