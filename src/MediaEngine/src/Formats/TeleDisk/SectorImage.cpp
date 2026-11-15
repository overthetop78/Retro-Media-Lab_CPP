#include "Internal.h"
#include "Constants.h"
#include "../Raw/RawImages.h"
#include "../../Models/Sectors/UniformLayout.h"
#include <RetroMediaLab/MediaFileSystems/Fat.h>
#include <RetroMediaLab/MediaFileSystems/FatBootSectorLayout.h>
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <algorithm>
#include <tuple>
namespace RetroMediaLab::MediaEngine::TeleDisk {
namespace M=MediaFileSystems;
std::shared_ptr<const M::SectorImage> buildSectorImage(const std::vector<Track>& tracks,std::uint8_t dataRate,std::stop_token token){
    using namespace Constants;struct Entry {const Track* track;const Sector* sector;std::int32_t order;};std::vector<Entry> all;std::vector<std::pair<std::uint32_t,std::size_t>> sizes;std::uint32_t cylinders{},heads{},spt{};
    for(const auto& track:tracks){M::checkCancellation(token);cylinders=std::max(cylinders,std::uint32_t(track.cylinder)+1);heads=std::max(heads,std::uint32_t(track.head&HeadMask)+1);spt=std::max(spt,std::uint32_t(track.sectors.size()));
        for(std::size_t order=0;order<track.sectors.size();++order){const auto& sector=track.sectors[order];const auto size=std::uint32_t(BaseSectorSize<<sector.sizeCode);auto found=std::find_if(sizes.begin(),sizes.end(),[&](const auto& pair){return pair.first==size;});if(found==sizes.end())sizes.emplace_back(size,1);else ++found->second;all.push_back({&track,&sector,std::int32_t(order)});}
    }
    const auto blockSize=std::max_element(sizes.begin(),sizes.end(),[](const auto& a,const auto& b){return a.second<b.second;})->first;
    std::vector<Entry> ordered;for(const auto& entry:all)if((BaseSectorSize<<entry.sector->sizeCode)==blockSize)ordered.push_back(entry);
    std::stable_sort(ordered.begin(),ordered.end(),[](const auto& a,const auto& b){return std::tuple(a.track->cylinder,a.track->head&HeadMask,a.sector->number)<std::tuple(b.track->cylinder,b.track->head&HeadMask,b.sector->number);});
    std::vector<M::SectorBlock> blocks;
    for(std::size_t logical=0;logical<ordered.size();++logical){M::checkCancellation(token);const auto& entry=ordered[logical];const auto& sector=*entry.sector;if(!sector.data)continue;
        M::SectorBlock block;block.logicalBlock=logical;block.address={entry.track->cylinder,std::int64_t(entry.track->head&HeadMask),sector.number};block.data=*sector.data;block.integrityValid=!(sector.flags&DataCrcError);block.dataIntegrityValid=block.integrityValid;block.deletedDataMark=bool(sector.flags&DeletedData);block.formatCode=dataRate;block.diagnosticCode=sector.flags;block.recordedAddress=M::SectorAddress{sector.cylinder,sector.head,sector.number};block.recordedSizeCode=sector.sizeCode;block.recordedTrackOrder=entry.order;blocks.push_back(std::move(block));
    }
    std::string format(M::Formats::UcsdIbmMfm);const auto imageLength=std::uint64_t(cylinders)*heads*spt*blockSize;
    const auto boot=std::find_if(blocks.begin(),blocks.end(),[](const auto& b){return b.address.cylinder==M::FatBootSectorLayout::SystemCylinder&&b.address.head==M::FatBootSectorLayout::SystemHead&&b.address.number==M::FatBootSectorLayout::BootSectorNumber;});
    const bool bpb=boot!=blocks.end()&&M::Fat::detectBpbGeometry(boot->data,imageLength).has_value();const bool jump=boot!=blocks.end()&&!boot->data.empty()&&(boot->data.front()==M::FatBootSectorLayout::ShortJumpOpcode||boot->data.front()==M::FatBootSectorLayout::NearJumpOpcode);
    if((bpb||jump)&&blockSize==M::FatBootSectorLayout::SectorSize){const auto geometry=Raw::ibmFromCapacity(imageLength);format=geometry&&geometry->cylinders==cylinders&&geometry->heads==heads&&geometry->sectorsPerTrack==spt?geometry->formatId:std::string(M::Formats::IbmScan);}
    M::SectorGeometry geometry{format,blockSize,cylinders,heads,spt};
    if(ordered.size()==all.size())if(auto uniform=tryUniformSectorImage(geometry,blocks,token))return uniform;
    M::SectorImageOptions options;options.capacity=std::uint64_t(ordered.size())*blockSize;options.logicalBlockCount=ordered.size();return sparseSectorImage(std::move(geometry),std::move(blocks),options);
}
}
