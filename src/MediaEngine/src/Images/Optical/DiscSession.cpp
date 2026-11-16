#include <RetroMediaLab/MediaEngine/OpticalDiscSession.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "Constants.h"
#include "../MediaDocument.h"
#include "../../Models/Optical/Image.h"
#include "../../Models/Optical/Internal.h"
#include <QString>
#include <mutex>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace OpticalSessionConstants;
template<class Map>void loop(OpticalDiscTrack& output,const Map& values){const auto no=values.find(std::string(NoLoopKey)),yes=values.find(std::string(LoopKey));output.loopMode=no!=values.end()?LoopDisabled:yes!=values.end()?LoopEnabled:0;if(yes!=values.end())output.loopOffset=QString::fromUtf8(yes->second).toInt();}
OpticalDiscTrack summary(const OpticalTrack& track,const std::map<std::string,std::string>* fallback){const auto& v=track.options();OpticalDiscTrack result{v.trackNumber,v.mode,v.firstSector,v.sectorCount,v.storedSectorSize,v.userDataOffset,v.userDataLength,v.pregapSectors,v.postgapSectors,v.storedPregapSectors,bool(v.subchannelSource)};
    if(v.metadata.contains(std::string(LoopKey))||v.metadata.contains(std::string(NoLoopKey))||!fallback)loop(result,v.metadata);else loop(result,*fallback);return result;
}
std::size_t bufferSize(std::int64_t count,int size){const auto bytes=OpticalInternal::multiply(count,size);if(bytes>MaximumCacheBytes)throw MediaError(DiagnosticCode::OpticalSessionCache);return std::size_t(bytes);}
}
struct OpticalDiscSession::State {
    std::mutex mutex;bool closed=false;std::shared_ptr<const MediaDocument> document;std::vector<std::shared_ptr<const OpticalTrack>> tracks;std::vector<OpticalDiscTrack> summaries;std::string format;
    std::optional<std::vector<std::vector<std::uint8_t>>> cachedSectors;
    std::vector<std::optional<std::vector<std::uint8_t>>> cachedSubchannels;
    void requireOpen()const{if(closed)throw MediaError(DiagnosticCode::OpticalSessionClosed);}
    const OpticalTrack& requireTrack(int index)const{if(index<0||std::size_t(index)>=tracks.size())throw MediaError(DiagnosticCode::OpticalSessionRange);return *tracks[std::size_t(index)];}
};
OpticalDiscSession::OpticalDiscSession(std::shared_ptr<const MediaDocument> document):OpticalDiscSession(std::move(document),true){}
OpticalDiscSession::OpticalDiscSession(std::shared_ptr<const MediaDocument> document,bool resetOnFailure){const auto drive=document?document->mediaSource()->options().drive:nullptr;try{state_=std::make_shared<State>();state_->document=std::move(document);if(!state_->document)throw MediaError(DiagnosticCode::OpticalSessionImage);const auto* image=dynamic_cast<const OpticalImage*>(&state_->document->representation());if(!image||!image->options().tracks||image->options().tracks->empty())throw MediaError(DiagnosticCode::OpticalSessionImage);state_->format=state_->document->formatId();state_->tracks=*image->options().tracks;std::stable_sort(state_->tracks.begin(),state_->tracks.end(),[](const auto& a,const auto& b){return a->trackNumber()<b->trackNumber();});for(std::size_t i=0;i<state_->tracks.size();++i)state_->summaries.push_back(summary(*state_->tracks[i],i==0?&state_->document->metadata():nullptr));}
    catch(...){state_.reset();if(resetOnFailure&&drive){try{drive->reset();}catch(...){}}throw;}
}
OpticalDiscSession::~OpticalDiscSession()noexcept{try{close();}catch(...){}}
std::vector<OpticalDiscTrack> OpticalDiscSession::tracks()const{return state_->summaries;}
std::string OpticalDiscSession::formatId()const{return state_->format;}
bool OpticalDiscSession::audioBigEndian()const{return state_->format==OpticalSessionConstants::ChdFormat;}
std::future<void> OpticalDiscSession::readStoredSectorAsync(int index,std::int64_t sector,std::span<std::uint8_t> destination,std::stop_token token)const{return std::async(std::launch::deferred,[state=state_,index,sector,destination,token]{std::lock_guard lock(state->mutex);state->requireOpen();checkCancellation(token);const auto& track=state->requireTrack(index);const auto& v=track.options();if(state->cachedSectors){const auto stored=OpticalInternal::add(sector,v.storedPregapSectors),count=OpticalInternal::add(v.sectorCount,v.storedPregapSectors);if(stored<0||stored>=count||destination.size()!=std::size_t(v.storedSectorSize))throw MediaError(DiagnosticCode::OpticalSessionRange);const auto& data=(*state->cachedSectors)[std::size_t(index)];std::copy_n(data.data()+std::size_t(OpticalInternal::multiply(stored,v.storedSectorSize)),destination.size(),destination.data());}else track.readStoredSectorAsync(sector,destination,token).get();});}
std::future<std::optional<std::vector<std::uint8_t>>> OpticalDiscSession::readSubchannelAsync(int index,std::int64_t sector,std::stop_token token)const{return std::async(std::launch::deferred,[state=state_,index,sector,token]()->std::optional<std::vector<std::uint8_t>>{std::lock_guard lock(state->mutex);state->requireOpen();checkCancellation(token);const auto& track=state->requireTrack(index);if(!state->cachedSectors)return track.readSubchannelAsync(sector,token).get();const auto& data=state->cachedSubchannels[std::size_t(index)];if(!data)return {};if(sector<0||sector>=track.sectorCount())throw MediaError(DiagnosticCode::OpticalSessionRange);const auto start=std::size_t(OpticalInternal::multiply(sector,OpticalSessionConstants::SubBytes));return std::vector<std::uint8_t>(data->begin()+start,data->begin()+start+OpticalSessionConstants::SubBytes);});}
std::future<void> OpticalDiscSession::precacheAsync(std::stop_token token)const{return std::async(std::launch::deferred,[state=state_,token]{using namespace OpticalSessionConstants;std::lock_guard lock(state->mutex);state->requireOpen();if(state->cachedSectors)return;std::vector<std::vector<std::uint8_t>> sectors(state->tracks.size());std::vector<std::optional<std::vector<std::uint8_t>>> subchannels(state->tracks.size());for(std::size_t i=0;i<state->tracks.size();++i){checkCancellation(token);const auto& track=*state->tracks[i];const auto& v=track.options();const auto count=OpticalInternal::add(v.sectorCount,v.storedPregapSectors);sectors[i].resize(bufferSize(count,v.storedSectorSize));if(v.subchannelSource)subchannels[i]=std::vector<std::uint8_t>(bufferSize(v.sectorCount,SubBytes));for(std::int64_t sector=-v.storedPregapSectors;sector<v.sectorCount;++sector){checkCancellation(token);const auto offset=std::size_t(OpticalInternal::multiply(sector+v.storedPregapSectors,v.storedSectorSize));auto output=std::span(sectors[i]).subspan(offset,v.storedSectorSize);track.readStoredSectorAsync(sector,output,token).get();if(sector>=0&&subchannels[i]){const auto bytes=track.readSubchannelAsync(sector,token).get();if(bytes){if(bytes->size()>SubBytes)throw MediaError(DiagnosticCode::OpticalSessionImage);std::copy(bytes->begin(),bytes->end(),subchannels[i]->begin()+std::size_t(OpticalInternal::multiply(sector,SubBytes)));}}}}
    state->cachedSubchannels=std::move(subchannels);state->cachedSectors=std::move(sectors);
});}
void OpticalDiscSession::close(){if(!state_)return;std::shared_ptr<DriveMechanics> drive;{std::lock_guard lock(state_->mutex);if(state_->closed)return;state_->closed=true;if(state_->document)drive=state_->document->mediaSource()->options().drive;state_->document.reset();state_->tracks.clear();state_->cachedSectors.reset();state_->cachedSubchannels.clear();}if(drive)drive->reset();}
}
