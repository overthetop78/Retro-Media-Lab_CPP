#include "MfmReconstruction.h"
#include "../../Decoding/Candidates.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaFileSystems/CommodoreGeometry.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::CommodoreDisks::Internal {
namespace G=MediaFileSystems::Commodore;
namespace C=G::Constants;
void MfmReconstruction::add(Decoding::Result decoded,std::int32_t cylinder,std::int32_t head,std::int32_t revolution,std::stop_token token){
    checkCancellation(token);
    for(auto& sector:decoded.sectors){
        checkCancellation(token);
        if(!sector.data||sector.data->size()!=C::PhysicalSectorBytes1581||sector.number<C::FirstPhysicalSector1581||sector.number>C::PhysicalSectors1581)continue;
        const auto key=std::tuple{cylinder,head,sector.number};const auto found=lookup_.find(key);
        if(found!=lookup_.end()&&!Decoding::betterIntegrity(sector.integrityValid,candidates_[found->second].block.integrityValid))continue;
        MediaFileSystems::SectorBlock block;block.data=std::move(*sector.data);block.integrityValid=sector.integrityValid;block.revolution=revolution;
        if(found==lookup_.end()){lookup_.emplace(key,candidates_.size());candidates_.push_back({cylinder,head,sector.number,std::move(block)});}
        else candidates_[found->second].block=std::move(block);
    }
}
std::shared_ptr<const MediaFileSystems::SectorImage> MfmReconstruction::finish(std::stop_token token)&&{
    checkCancellation(token);if(candidates_.empty())throw MediaError(DiagnosticCode::ScpNo1581Sectors);
    std::vector<MediaFileSystems::SectorBlock> blocks;
    for(auto& candidate:candidates_){
        checkCancellation(token);if(candidate.cylinder<0||candidate.cylinder>=C::Cylinders1581||candidate.head<0||candidate.head>=C::PhysicalHeads1581)continue;
        const auto first=G::physicalBlock1581(candidate.cylinder,candidate.head,candidate.sector);
        for(std::int32_t half=0;half<C::BlocksPerPhysicalSector1581;++half){
            MediaFileSystems::SectorBlock block;block.logicalBlock=first+half;block.address={std::int64_t(block.logicalBlock/C::BlocksPerTrack1581),0,std::int64_t(block.logicalBlock%C::BlocksPerTrack1581)};
            const auto begin=candidate.block.data.begin()+half*C::SectorSize;block.data.assign(begin,begin+C::SectorSize);block.integrityValid=candidate.block.integrityValid;block.revolution=candidate.block.revolution;blocks.push_back(std::move(block));
        }
    }
    if(blocks.empty())throw MediaError(DiagnosticCode::ScpNoUsable1581Sectors);
    return sparseSectorImage({std::string(C::Format1581),C::SectorSize,C::Cylinders1581,C::Heads1581,C::BlocksPerTrack1581},std::move(blocks));
}
}
