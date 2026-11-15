#include "FluxReader.h"
#include "Constants.h"
#include "../../Codecs/Commodore/Commodore900.h"
#include "../../Decoding/Candidates.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <map>
namespace RetroMediaLab::MediaEngine::Commodore900 {
std::shared_ptr<const MediaFileSystems::SectorImage> readFlux(const Scp::Image& image,TrackProgress progress,std::stop_token token) {
    checkCancellation(token);
    std::map<std::uint64_t,MediaFileSystems::SectorBlock> best;
    std::size_t completed=0;
    for(const auto& track:image.tracks) {
        checkCancellation(token);
        if(track.cylinder>=0&&track.cylinder<Constants::CylinderCount&&track.head>=0&&track.head<Constants::HeadCount) {
            Scp::visitDecodeWindows(track,[&](const Scp::DecodeWindow& window){
                auto decoded=Commodore900Gcr::decode(window.flux,token);
                for(auto& sector:decoded.sectors) {
                    checkCancellation(token);
                    if(!sector.data||sector.data->size()!=Constants::SectorSize||sector.cylinder!=track.cylinder||sector.number<0||std::uint32_t(sector.number)>=sectorsPerTrack(track.cylinder))continue;
                    const MediaFileSystems::SectorAddress address{track.cylinder,track.head,sector.number};
                    const auto logical=logicalBlockOf(address);
                    const auto found=best.find(logical);
                    if(found!=best.end()&&!Decoding::betterIntegrity(sector.integrityValid,found->second.integrityValid))continue;
                    MediaFileSystems::SectorBlock block;
                    block.logicalBlock=logical;block.address=address;block.data=std::move(*sector.data);block.integrityValid=sector.integrityValid;block.revolution=window.revolution;
                    best.insert_or_assign(logical,std::move(block));
                }
            },token);
        }
        if(progress)progress(track,++completed,image.tracks.size());else ++completed;
    }
    if(best.empty())throw MediaError(DiagnosticCode::ScpNoC900Sectors);
    const auto count=best.rbegin()->first+1;
    std::vector<MediaFileSystems::SectorBlock> blocks;blocks.reserve(best.size());
    for(auto& [logical,block]:best)blocks.push_back(std::move(block));
    MediaFileSystems::SectorImageOptions options;options.capacity=count*Constants::SectorSize;options.logicalBlockCount=count;
    return sparseSectorImage(geometry(),std::move(blocks),options);
}
}
