#include "Scp.h"
#include "../../Models/Flux/ProtectedTrack.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Scp {
namespace {
template<class Get> void windows(std::size_t count,Get get,const std::function<void(const DecodeWindow&)>& visit,std::stop_token token) {
    for(std::size_t index=0;index<count;++index) {
        checkCancellation(token);
        const auto& current=get(index);
        DecodeWindow window{current,std::int32_t(index+1),false};
        if(index+1<count) {
            const auto& next=get(index+1);
            const auto joinedCount=std::uint64_t(current.intervals.size())+next.intervals.size();
            const auto ticks=std::uint64_t(current.indexTimeTicks)+next.indexTimeTicks;
            if(joinedCount>Constants::MaximumLength||ticks>Constants::MaximumInterval)throw MediaError(DiagnosticCode::ScpOverflow);
            window.flux.intervals.reserve(std::size_t(joinedCount));
            window.flux.intervals.insert(window.flux.intervals.end(),next.intervals.begin(),next.intervals.end());
            window.flux.indexTimeTicks=std::uint32_t(ticks);
            window.continuous=true;
        }
        checkCancellation(token);
        visit(window);
    }
}
}
void visitDecodeWindows(const Track& track,const std::function<void(const DecodeWindow&)>& visit,std::stop_token token){windows(track.revolutions.size(),[&](auto index)->const Flux::Revolution&{return track.revolutions[index].flux;},visit,token);}
void visitDecodeWindows(const Flux::ProtectedTrack& track,const std::function<void(const DecodeWindow&)>& visit,std::stop_token token){windows(track.revolutions().size(),[&](auto index)->const Flux::Revolution&{return track.revolutions()[index].flux();},visit,token);}
std::vector<DecodeWindow> decodeWindows(const Flux::ProtectedTrack& track,std::stop_token token){
    checkCancellation(token);std::vector<DecodeWindow> result;visitDecodeWindows(track,[&](const auto& window){result.push_back(window);},token);return result;
}
std::vector<DecodeWindow> decodeWindows(const Track& track,std::stop_token token) {
    checkCancellation(token);
    std::vector<DecodeWindow> result;
    visitDecodeWindows(track,[&](const auto& window){result.push_back(window);},token);
    return result;
}
DecodeWindow primaryWindow(const Track& track,std::stop_token token) {
    // Preserve the source's validation of every joined window, including overflow after the first.
    auto windows=decodeWindows(track,token);
    if(windows.empty())throw MediaError(DiagnosticCode::ScpNoRevolution);
    return std::move(windows.front());
}
}
