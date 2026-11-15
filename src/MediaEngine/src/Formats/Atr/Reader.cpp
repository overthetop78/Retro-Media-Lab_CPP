#include "Atr.h"
#include "Internal.h"
#include "../Atari8Bit/Geometry.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Atr {
std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes bytes,std::stop_token token){using namespace Constants;validate(bytes);const auto size=Internal::word(bytes,SectorSizeOffset);const auto payload=usablePayloadLength(bytes);const auto count=size==SingleSize?std::int32_t((payload+size-1)/size):sectorCount(std::int32_t(payload),size);auto geometry=Atari8Bit::geometry(size,count);std::vector<MediaFileSystems::SectorBlock> blocks;std::size_t offset=HeaderSize;
 for(std::int32_t sector=FirstSector;sector<=count;++sector){checkCancellation(token);const auto length=sector<=std::int32_t(BootCount)?SingleSize:size;const auto logical=std::uint32_t(sector-FirstSector);MediaFileSystems::SectorBlock b;b.logicalBlock=logical;b.address={logical/geometry.sectorsPerTrack,FirstHead,logical%geometry.sectorsPerTrack+FirstSector};b.data.resize(length,0);const auto available=std::min(std::size_t(length),bytes.size()-offset);if(available)std::copy_n(bytes.begin()+std::ptrdiff_t(offset),available,b.data.begin());offset+=available;blocks.push_back(std::move(b));}
 const auto truncated=truncatedSingleDensity(size,count);if(truncated)for(auto logical=std::uint32_t(count);logical<StandardCount;++logical){MediaFileSystems::SectorBlock b;b.logicalBlock=logical;b.address={logical/geometry.sectorsPerTrack,FirstHead,logical%geometry.sectorsPerTrack+FirstSector};b.data.resize(SingleSize,0);blocks.push_back(std::move(b));}
 MediaFileSystems::SectorImageOptions options;options.allowVariableBlockSize=size!=SingleSize;options.capacity=truncated?std::uint64_t(StandardCount)*SingleSize:payload;options.logicalBlockCount=truncated?StandardCount:std::uint32_t(count);return sparseSectorImage(std::move(geometry),std::move(blocks),options);
}
}
