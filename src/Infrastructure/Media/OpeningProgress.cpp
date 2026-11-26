#include "OpeningProgress.h"
#include "OpeningProgressConstants.h"
#include <algorithm>
namespace RetroMediaLab::Infrastructure {
namespace C=OpeningProgressConstants;
void OpeningProgress::reset(std::string name){const std::lock_guard lock(mutex_);value_={};value_.stage.resourceKey=C::Reading;value_.detail=std::move(name);value_.value=C::Initial;++value_.revision;}
LoadingProgress OpeningProgress::snapshot()const{const std::lock_guard lock(mutex_);return value_;}
void OpeningProgress::loadCapture(std::shared_ptr<const MediaEngine::DiskCaptureSnapshot> capture){if(!capture)return;const std::lock_guard lock(mutex_);value_.capture=capture;auto& progress=value_.tracks;progress={};for(const auto& track:capture->tracks){progress.cylinders.push_back(track.cylinder);progress.heads.push_back(track.head);}const auto unique=[](auto& list){std::sort(list.begin(),list.end());list.erase(std::unique(list.begin(),list.end()),list.end());};unique(progress.cylinders);unique(progress.heads);++value_.revision;}
void OpeningProgress::report(const MediaEngine::DiskExplorationProgress& item){
    using S=MediaEngine::DiskExplorationStage;const std::lock_guard lock(mutex_);const auto fraction=std::clamp(static_cast<double>(item.completed)/std::max<std::size_t>(1,item.total),C::Zero,C::Unit);
    double measured;const char* key;
    switch(item.kind){
    case S::FormatProbeStarted:case S::FormatProbeCompleted:measured=C::Initial+C::ProbeWeight*fraction;break;
    case S::RevolutionDecoded:case S::TrackDecoded:measured=C::DecodeStart+C::DecodeWeight*fraction;break;
    default:measured=C::CandidateStart+C::CandidateWeight*fraction;break;
    }
    switch(item.kind){
    case S::FormatProbeStarted:key=C::Testing;break;
    case S::CandidateStarted:key=C::ReadingFormat;break;
    case S::FormatProbeCompleted:case S::CandidateCompleted:key=item.recognized.value_or(false)?C::Recognized:C::Rejected;break;
    case S::RevolutionDecoded:key=C::Revolution;break;
    default:key=C::Track;break;
    }
    if(item.kind==S::TrackDecoded&&item.cylinder&&item.head&&!value_.tracks.cylinders.empty()){auto& track=value_.tracks;const auto coordinate=std::pair{*item.cylinder,*item.head};if(track.states[coordinate]!=GwTrackState::Success){track.states[coordinate]=GwTrackState::Success;++track.completed;++track.completedPerHead[*item.head];}track.cylinder=item.cylinder;track.head=item.head;}
    value_.stage={C::Recognition,{},{}};value_.detail=item.detail;value_.detailMessage=MediaEngine::MessageInfo{key,item.kind==S::RevolutionDecoded||item.kind==S::TrackDecoded?std::vector<std::string>{std::to_string(item.completed),std::to_string(item.total)}:std::vector<std::string>{item.detail},{}};
    if (item.kind == S::CandidateStarted || item.kind == S::CandidateCompleted ||
        item.kind == S::FormatProbeStarted || item.kind == S::FormatProbeCompleted)
        value_.formatCheck = value_.detailMessage;
    value_.value=std::max(value_.value,measured);value_.indeterminate=item.total==0;++value_.revision;
}
void OpeningProgress::report(const MediaEngine::MediaExplorationProgress& item){
    using S=MediaEngine::MediaExplorationStage;const std::lock_guard lock(mutex_);const bool tape=item.mediaKind==MediaEngine::MediaKind::Tape;
    const auto* key=item.stage==S::ReadingMedia?C::Reading:item.stage==S::ReadingFileSystem?(tape?C::TapeDecode:C::FileSystem):(tape?C::Files:C::Contents);
    value_.stage={key,{},{}};value_.detail=item.detail;value_.detailMessage=item.message;value_.value=std::clamp(std::max(value_.value,item.value),C::Zero,C::Maximum);value_.indeterminate=false;++value_.revision;
}
}
