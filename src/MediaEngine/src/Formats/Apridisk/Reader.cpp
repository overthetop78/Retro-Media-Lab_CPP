#include "Internal.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
#include <map>
namespace RetroMediaLab::MediaEngine::Apridisk {
namespace {
using namespace Constants;
void available(MediaFileSystems::Bytes bytes,std::size_t offset,std::size_t count){if(offset>bytes.size()||count>bytes.size()-offset)throw MediaError(DiagnosticCode::ApridiskTruncated,{std::to_string(offset)});}
std::vector<std::uint8_t> decode(MediaFileSystems::Bytes bytes,std::uint16_t compression){
    if(compression==Uncompressed){if(bytes.size()!=BlockSize)throw MediaError(DiagnosticCode::ApridiskSectorLength,{std::to_string(bytes.size()),std::to_string(BlockSize)});return {bytes.begin(),bytes.end()};}
    if(compression!=Compressed||bytes.size()%RunSize)throw MediaError(DiagnosticCode::ApridiskCompression,{std::to_string(compression)});
    std::vector<std::uint8_t> output(BlockSize);std::size_t written=0;
    for(std::size_t at=0;at<bytes.size();at+=RunSize){const auto count=Internal::little<std::uint16_t>(bytes,at);if(count==0||count>output.size()-written)throw MediaError(DiagnosticCode::ApridiskExpansion);std::fill_n(output.begin()+std::ptrdiff_t(written),count,bytes[at+RunValueOffset]);written+=count;}
    if(written!=BlockSize)throw MediaError(DiagnosticCode::ApridiskSectorLength,{std::to_string(written),std::to_string(BlockSize)});return output;
}
}
MediaFileSystems::SectorGeometry geometry(){using namespace Constants;return {std::string(FormatId),BlockSize,Cylinders,Heads,SectorsPerTrack};}
std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes bytes,std::stop_token token){
    using namespace Constants;if(bytes.size()<HeaderSize||!std::equal(Signature.begin(),Signature.end(),bytes.begin()))throw MediaError(DiagnosticCode::ApridiskHeader);
    std::map<std::uint64_t,MediaFileSystems::SectorBlock> sectors;std::size_t offset=HeaderSize;
    while(offset<bytes.size()){
        MediaFileSystems::checkCancellation(token);available(bytes,offset,RecordHeaderSize);const auto type=Internal::little<std::uint32_t>(bytes,offset),payload=Internal::little<std::uint32_t>(bytes,offset+PayloadLengthOffset);const auto compression=Internal::little<std::uint16_t>(bytes,offset+CompressionOffset),header=Internal::little<std::uint16_t>(bytes,offset+HeaderLengthOffset),cylinder=Internal::little<std::uint16_t>(bytes,offset+CylinderOffset);const auto head=bytes[offset+HeadOffset],sector=bytes[offset+SectorOffset];
        if(header<RecordHeaderSize||payload>std::uint32_t(std::numeric_limits<std::int32_t>::max()))throw MediaError(DiagnosticCode::ApridiskRecord,{std::to_string(offset)});
        const auto record=std::uint64_t(header)+payload;if(record>std::uint64_t(std::numeric_limits<std::int32_t>::max()))throw MediaError(DiagnosticCode::ApridiskOverflow);available(bytes,offset,std::size_t(record));
        if(type==SectorRecord){if(cylinder>=Cylinders||head>=Heads||sector<1||sector>SectorsPerTrack)throw MediaError(DiagnosticCode::ApridiskAddress,{std::to_string(cylinder),std::to_string(head),std::to_string(sector)});const auto logical=(std::uint64_t(cylinder)*Heads+head)*SectorsPerTrack+sector-1;MediaFileSystems::SectorBlock block;block.logicalBlock=logical;block.address={cylinder,head,sector};block.data=decode(bytes.subspan(offset+header,payload),compression);sectors[logical]=std::move(block);}
        offset+=std::size_t(record);
    }
    if(sectors.size()!=BlockCount)throw MediaError(DiagnosticCode::ApridiskCount,{std::to_string(sectors.size()),std::to_string(BlockCount)});
    std::vector<MediaFileSystems::SectorBlock> blocks;for(auto& [logical,block]:sectors)blocks.push_back(std::move(block));return sparseSectorImage(geometry(),std::move(blocks));
}
}
