#include "ProtectedTrack.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <map>
namespace RetroMediaLab::MediaEngine::Flux {
ProtectedTrackImage::ProtectedTrackImage(std::vector<ProtectedTrack> tracks,bool writeProtected):tracks_(std::move(tracks)),writeProtected_(writeProtected){
    if(tracks_.empty())throw MediaError(DiagnosticCode::ProtectedImageEmpty);
    std::map<std::pair<std::int32_t,std::int32_t>,std::size_t> counts;
    for(const auto& track:tracks_)++counts[{track.cylinder(),track.head()}];
    for(const auto& track:tracks_)if(counts.at({track.cylinder(),track.head()})>1)throw MediaError(DiagnosticCode::ProtectedImageDuplicate,{std::to_string(track.cylinder()),std::to_string(track.head())});
    std::sort(tracks_.begin(),tracks_.end(),[](const auto& a,const auto& b){return std::pair{a.cylinder(),a.head()}<std::pair{b.cylinder(),b.head()};});
}
}
