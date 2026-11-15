#include "Msa.h"
#include "Constants.h"
#include "../St/Geometry.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Msa {
namespace {std::uint16_t word(MediaFileSystems::Bytes source,std::size_t offset){return std::uint16_t((std::uint16_t(source[offset])<<Constants::BitsPerByte)|source[offset+1]);}}
std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes source,std::stop_token token){using namespace Constants;if(source.size()<HeaderSize||word(source,SignatureOffset)!=Signature)throw MediaError(DiagnosticCode::MsaHeader,{std::to_string(source.size())});const std::uint32_t sectors=word(source,SectorsOffset),heads=std::uint32_t(word(source,HeadsOffset))+1,start=word(source,StartOffset),end=word(source,EndOffset);if(sectors<MinimumSectors||sectors>MaximumSectors||heads<MinimumHeads||heads>MaximumHeads||end<start||end>MaximumCylinder)throw MediaError(DiagnosticCode::MsaGeometry,{std::to_string(sectors),std::to_string(heads),std::to_string(start),std::to_string(end)});const auto trackBytes=sectors*SectorSize;std::size_t position=HeaderSize;std::vector<MediaFileSystems::SectorBlock> blocks;
 for(std::uint32_t cylinder=start;cylinder<=end;++cylinder)for(std::uint32_t head=0;head<heads;++head){checkCancellation(token);if(source.size()-position<WordSize)throw MediaError(DiagnosticCode::MsaTruncatedTable,{std::to_string(cylinder),std::to_string(head),std::to_string(position),std::to_string(source.size()-position)});const auto length=word(source,position);position+=WordSize;if(length>source.size()-position)throw MediaError(DiagnosticCode::MsaTruncatedTrack,{std::to_string(cylinder),std::to_string(head),std::to_string(position),std::to_string(length),std::to_string(source.size()-position)});auto packed=source.subspan(position,length);const auto track=length==trackBytes?std::vector<std::uint8_t>(packed.begin(),packed.end()):unpack(packed,std::int32_t(trackBytes),std::int32_t(cylinder),std::int32_t(head));position+=length;
  for(std::uint32_t sector=0;sector<sectors;++sector){MediaFileSystems::SectorBlock b;b.logicalBlock=(cylinder*heads+head)*sectors+sector;b.address={cylinder,head,sector+FirstSector};b.data.assign(track.begin()+sector*SectorSize,track.begin()+(sector+1)*SectorSize);blocks.push_back(std::move(b));}}
 const auto count=std::uint64_t(end+1)*heads*sectors;return sparseSectorImage({St::formatId(std::int64_t(count*SectorSize)),SectorSize,std::uint32_t(end+1),heads,sectors},std::move(blocks));
}
}
