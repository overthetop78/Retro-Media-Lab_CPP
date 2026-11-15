#include "Rx02.h"
#include "SectorOrder.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Rx02 {
MediaFileSystems::SectorGeometry geometry(){using namespace Constants;return {std::string(FormatId),LogicalSize,Tracks,Heads,LogicalPerTrack};}
std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes bytes,std::stop_token token){using namespace Constants;if(bytes.size()!=Capacity)throw MediaError(DiagnosticCode::Rx02ImageLength,{std::to_string(bytes.size()),std::to_string(Capacity)});std::vector<MediaFileSystems::SectorBlock> blocks;blocks.reserve(LogicalCount);
    for(std::uint32_t logical=0;logical<LogicalCount;++logical){MediaFileSystems::checkCancellation(token);MediaFileSystems::SectorBlock block;block.logicalBlock=logical;block.address={logical/LogicalPerTrack,0,logical%LogicalPerTrack+FirstSector};for(std::uint32_t part=0;part<Parts;++part){const auto offset=physicalOffset(logical*Parts+part);block.data.insert(block.data.end(),bytes.begin()+std::ptrdiff_t(offset),bytes.begin()+std::ptrdiff_t(offset+PhysicalSize));}blocks.push_back(std::move(block));}
    return sparseSectorImage(geometry(),std::move(blocks));
}
}
