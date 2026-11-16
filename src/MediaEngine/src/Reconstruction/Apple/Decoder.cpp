#include "Decoder.h"
#include "Constants.h"
#include "../Iso/Builder.h"
#include "../../Codecs/AppleIwm/Gcr.h"
#include "../../Formats/Apple/Geometry.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <limits>
#include <set>
namespace RetroMediaLab::MediaEngine::AppleScp {
Decoder::Decoder(Decoding::DecodeOperation decode,MacDecode mac):decode_(std::move(decode)),mac_(std::move(mac)){
    if(!mac_)mac_=[](const auto& revolution,double cell,auto token){return AppleIwm::decodeAtBitCell(revolution,cell,AppleIwm::Variant::Macintosh,token);};
}
Decoding::Result Decoder::decodeMacTrack(const Scp::Track& track,const Flux::Revolution& revolution,std::stop_token token)const{
    const auto expected=Apple::Geometry::MacintoshGcr::sectors(track.cylinder);const auto initial=Flux::estimateCell(revolution.intervals,Flux::TimingMode::NonFm,token)*Constants::CellMultiplier;
    std::optional<Decoding::Result> best;auto bestScore=std::numeric_limits<std::int64_t>::min();
    for(const auto factor:Constants::CellFactors){
        checkCancellation(token);auto candidate=mac_(revolution,initial*factor,token);std::set<std::int32_t> distinct,valid;std::int64_t validCount=0,count=0;
        for(const auto& sector:candidate.sectors)if(sector.data&&sector.data->size()==AppleIwm::Constants::SectorByteCount&&sector.cylinder==track.cylinder&&sector.head==track.head&&sector.number>=0&&sector.number<expected){distinct.insert(sector.number);++count;if(sector.integrityValid==true){valid.insert(sector.number);++validCount;}}
        const auto score=static_cast<std::int64_t>(distinct.size())*Constants::DistinctWeight+validCount*Constants::ValidWeight+count;
        if(score>bestScore){best=std::move(candidate);bestScore=score;}if(valid.size()==static_cast<std::size_t>(expected))break;
    }
    return std::move(*best);
}
IsoReconstruction::Candidates Decoder::collect(const Scp::Image& image,std::string_view id,std::int32_t size,const IsoReconstruction::CollectionProgressCallback& progress,std::stop_token token)const{
    checkCancellation(token);if(image.tracks.size()>static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))throw MediaError(DiagnosticCode::ScpOverflow);
    IsoReconstruction::Candidates result;std::int32_t completed=0;
    for(const auto& track:image.tracks){
        checkCancellation(token);const bool mac=id==AppleIwm::Constants::MacId;
        if(!mac||(track.cylinder>=0&&track.cylinder<Apple::Geometry::MacintoshGcr::CylinderCount))Scp::visitDecodeWindows(track,[&](const auto& window){
            if(!mac&&!decode_)throw MediaError(DiagnosticCode::IsoDecoderMissing);
            const auto decoded=mac?decodeMacTrack(track,window.flux,token):decode_(id,std::make_shared<const Flux::Revolution>(window.flux));
            for(const auto& sector:decoded.sectors){checkCancellation(token);if(sector.data&&sector.data->size()==static_cast<std::size_t>(size))result.add({sector.cylinder,sector.head,sector.number},{sector,window.revolution,{}});}
        },token);
        ++completed;if(progress)progress({IsoReconstruction::CollectionProgressKind::TrackDecoded,completed,static_cast<std::int32_t>(image.tracks.size()),track.cylinder,track.head,0,static_cast<std::int32_t>(track.revolutions.size())});
    }
    return result;
}
MediaFileSystems::SectorBlock select(std::uint64_t logical,MediaFileSystems::SectorAddress address,std::span<const IsoReconstruction::Candidate> values,std::stop_token token){
    const auto& best=IsoReconstruction::best(values,token);MediaFileSystems::SectorBlock result;result.logicalBlock=logical;result.address=address;result.data=*best.sector.data;result.integrityValid=best.sector.integrityValid;result.revolution=best.revolution;result.tag=best.sector.tag;result.formatCode=best.sector.formatCode;return result;
}
}
