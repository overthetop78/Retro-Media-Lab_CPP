#include "FluxReader.h"
#include "GcrReconstruction.h"
#include "MfmReconstruction.h"
#include "../../Codecs/Iso/Mfm.h"
#include "../../Codecs/Commodore/Commodore1541.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::CommodoreDisks {
std::shared_ptr<const MediaFileSystems::SectorImage> readGcrFlux(const Scp::Image& image,std::optional<std::string_view> requested,TrackProgress progress,std::stop_token token){
    checkCancellation(token);Internal::GcrReconstruction reconstruction;std::size_t completed=0;
    for(const auto& track:image.tracks){
        checkCancellation(token);
        Scp::visitDecodeWindows(track,[&](const Scp::DecodeWindow& window){reconstruction.add(Commodore1541Gcr::decode(window.flux,token),window.revolution,token);},token);
        ++completed;if(progress)progress(track,completed,image.tracks.size());
    }
    const auto secondHead=std::any_of(image.tracks.begin(),image.tracks.end(),[](const auto& track){return track.head==1;});
    return std::move(reconstruction).finish(requested,secondHead,token);
}
}

namespace RetroMediaLab::MediaEngine::CommodoreDisks {
std::shared_ptr<const MediaFileSystems::SectorImage> readMfmFlux(const Scp::Image& image,TrackProgress progress,std::stop_token token){
    checkCancellation(token);Internal::MfmReconstruction reconstruction;std::size_t completed=0;
    for(const auto& track:image.tracks){
        checkCancellation(token);
        Scp::visitDecodeWindows(track,[&](const Scp::DecodeWindow& window){reconstruction.add(IsoMfm::decode(window.flux,token),track.cylinder,track.head,window.revolution,token);},token);
        ++completed;if(progress)progress(track,completed,image.tracks.size());
    }
    return std::move(reconstruction).finish(token);
}
}
