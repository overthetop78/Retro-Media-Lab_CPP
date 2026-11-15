#include "GcrReconstruction.h"
#include "../../Codecs/Commodore/Commodore1541Constants.h"
#include "../../Decoding/Candidates.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaFileSystems/CommodoreGeometry.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::CommodoreDisks::Internal {
namespace G=MediaFileSystems::Commodore;
namespace C=G::Constants;
void GcrReconstruction::add(Decoding::Result decoded,std::int32_t revolution,std::stop_token token){
    for(auto& sector:decoded.sectors){
        checkCancellation(token);
        if(!sector.data||sector.cylinder<Commodore1541Gcr::Constants::MinimumDiskTrack||sector.cylinder>Commodore1541Gcr::Constants::MaximumDiskTrack||sector.number<0)continue;
        maxTrack_=std::max(maxTrack_,std::int32_t(sector.cylinder));
        if(sector.cylinder>C::StandardTracks&&sector.cylinder<=C::ExtendedTracks&&std::any_of(sector.data->begin(),sector.data->end(),[](auto value){return value!=0;}))extendedData_=true;
        const auto key=std::pair{std::int32_t(sector.cylinder),sector.number};const auto found=lookup_.find(key);
        if(found!=lookup_.end()&&!Decoding::betterIntegrity(sector.integrityValid,candidates_[found->second].block.integrityValid))continue;
        MediaFileSystems::SectorBlock block;block.data=std::move(*sector.data);block.integrityValid=sector.integrityValid;block.revolution=revolution;
        if(found==lookup_.end()){lookup_.emplace(key,candidates_.size());candidates_.push_back({key.first,key.second,std::move(block)});}
        else candidates_[found->second].block=std::move(block);
    }
}
std::shared_ptr<const MediaFileSystems::SectorImage> GcrReconstruction::finish(std::optional<std::string_view> requested,bool physicalSecondHead,std::stop_token token)&&{
    checkCancellation(token);if(candidates_.empty())throw MediaError(DiagnosticCode::ScpNoCommodoreSectors);
    const auto twoSides=(requested&&*requested==C::Format1571)||maxTrack_>C::ExtendedTracks||physicalSecondHead;
    const auto tracks=twoSides?C::StandardTracks:maxTrack_>C::StandardTracks&&extendedData_?C::ExtendedTracks:C::StandardTracks;
    const auto sides=twoSides?C::TwoSides:1;const auto count=G::blocksPerSide1541(tracks)*sides;
    std::vector<MediaFileSystems::SectorBlock> blocks;blocks.reserve(candidates_.size());
    for(auto& candidate:candidates_){
        checkCancellation(token);const auto side=candidate.track>tracks?1:0;const auto track=side==0?candidate.track:candidate.track-tracks;
        if(side>=sides||track<C::FirstTrack||track>tracks||candidate.sector>=G::sectors1541(track))continue;
        auto& block=candidate.block;
        block.logicalBlock=twoSides?G::block1571(track,candidate.sector,tracks,side):G::sideBlock1541(track,candidate.sector,tracks);
        block.address={track-1,side,candidate.sector};blocks.push_back(std::move(block));
    }
    MediaFileSystems::SectorImageOptions options;options.logicalBlockCount=count;options.capacity=std::uint64_t(count)*C::SectorSize;
    return sparseSectorImage({std::string(twoSides?C::Format1571:C::Format1541),C::SectorSize,std::uint32_t(tracks),std::uint32_t(sides),C::MaximumSectors},std::move(blocks),options);
}
}
