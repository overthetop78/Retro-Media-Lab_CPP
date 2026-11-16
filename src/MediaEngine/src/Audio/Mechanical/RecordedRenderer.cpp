#include "RecordedRenderer.h"
#include "../../Formats/Tape/Common/Pcm.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <cmath>
#include <future>
#include <map>
#include <mutex>
#include <limits>
namespace RetroMediaLab::MediaEngine::MechanicalAudio {
namespace {
template<class Key,class Value> class Cache {
    std::mutex mutex_;
    std::map<Key,std::shared_future<Value>> values_;
public:
    template<class Load> Value get(const Key& key,Load load){
        std::shared_future<Value> future;
        std::optional<std::promise<Value>> producer;
        {
            std::lock_guard lock(mutex_);
            if(const auto found=values_.find(key);found!=values_.end())future=found->second;
            else {producer.emplace();future=producer->get_future().share();values_.emplace(key,future);}
        }
        if(producer){try{producer->set_value(load());}catch(...){producer->set_exception(std::current_exception());}}
        return future.get();
    }
};
using Samples=std::shared_ptr<const std::vector<float>>;
}
struct RecordedRenderer::State {
    std::vector<RecordedProfile> profiles;
    SampleLoader loader;DurationLoader durationLoader;
    Cache<std::pair<std::string,int>,Samples> samples;
    Cache<std::string,double> durations;
    const RecordedProfile& profile(std::string_view id)const{
        const auto found=std::find_if(profiles.begin(),profiles.end(),[&](const auto& value){return value.id==id;});
        if(found==profiles.end())throw MediaError(DiagnosticCode::AudioPcmData);return *found;
    }
    const RecordedSample* select(const RecordedProfile& value,std::string_view operation,int distance,int direction,int cylinder)const{
        if(const auto* selected=selectRecordedSampleInProfile(value,operation,distance,direction,cylinder))return selected;
        const auto base=baseRecordedProfile(value.device);
        if(base&&*base!=value.id){const auto found=std::find_if(profiles.begin(),profiles.end(),[&](const auto& p){return p.id==*base;});
            if(found!=profiles.end())return select(*found,operation,distance,direction,cylinder);}
        return nullptr;
    }
    Samples load(const RecordedSample& sample,int rate){return samples.get({std::string(sample.id()),rate},[&]{
        auto value=loader(sample,rate);
        if(std::any_of(value.begin(),value.end(),[](float item){return !std::isfinite(item);}))throw MediaError(DiagnosticCode::AudioPcmData);
        return std::make_shared<const std::vector<float>>(std::move(value));
    });}
};
RecordedRenderer::RecordedRenderer(std::vector<RecordedProfile> profiles,SampleLoader loader,DurationLoader durations):state_(std::make_unique<State>()){
    if(!loader||!durations)throw MediaError(DiagnosticCode::AudioPcmData);
    for(std::size_t i=0;i<profiles.size();++i)for(std::size_t j=0;j<i;++j)if(profiles[i].id==profiles[j].id)throw MediaError(DiagnosticCode::AudioPcmData);
    state_->profiles=std::move(profiles);state_->loader=std::move(loader);state_->durationLoader=std::move(durations);
}
RecordedRenderer::~RecordedRenderer()=default;
double RecordedRenderer::duration(std::string_view profile,std::string_view operation,int distance,int direction,int cylinder){
    const auto* sample=state_->select(state_->profile(profile),operation,distance,direction,cylinder);if(!sample)return 0;
    return state_->durations.get(std::string(sample->id()),[&]{const auto value=state_->durationLoader(*sample);if(!std::isfinite(value)||value<0)throw MediaError(DiagnosticCode::AudioPcmData);return value;});
}
void RecordedRenderer::prepareProfile(std::string_view id,int rate){
    if(rate<=0)throw MediaError(DiagnosticCode::AudioPcmData);
    const auto& profile=state_->profile(id);for(const auto& sample:profile.samples)state_->load(sample,rate);
    const auto base=baseRecordedProfile(profile.device);if(base&&*base!=id&&std::any_of(state_->profiles.begin(),state_->profiles.end(),[&](const auto& p){return p.id==*base;}))prepareProfile(*base,rate);
}
std::vector<std::int16_t> RecordedRenderer::render(std::string_view device,std::string_view operation,int rate,int count,std::int64_t offset,std::string_view id,double volume,int distance,int direction,int cylinder){
    if(rate<=0||count<0||offset<0||offset>std::numeric_limits<std::int64_t>::max()-count||!std::isfinite(volume)||volume<0||volume>Constants::MaximumVolumePercent)throw MediaError(DiagnosticCode::AudioPcmData);
    const auto& profile=state_->profile(id);if(profile.device!=device)throw MediaError(DiagnosticCode::AudioPcmData);
    std::vector<std::int16_t> result(static_cast<std::size_t>(count));
    const auto* sample=state_->select(profile,operation,distance,direction,cylinder);if(!sample||volume==0)return result;
    const auto source=state_->load(*sample,rate);if(source->empty())return result;
    const auto action=std::find_if(sample->operations.begin(),sample->operations.end(),[&](const auto& value){return value.operation==operation;});
    const auto* beginning=action->loop?state_->select(profile,action->startOperation?std::string_view(*action->startOperation):Constants::Start,distance,direction,cylinder):nullptr;
    const auto start=beginning?state_->load(*beginning,rate):Samples{};
    const auto startCount=start?start->size():0;
    const auto gain=volume/Constants::MaximumVolumePercent;
    for(int index=0;index<count;++index){
        auto position=static_cast<std::uint64_t>(offset+index);double value;
        if(position<startCount)value=(*start)[static_cast<std::size_t>(position)];
        else {position-=startCount;if(!action->loop&&position>=source->size())break;value=(*source)[static_cast<std::size_t>(action->loop?position%source->size():position)];}
        result[static_cast<std::size_t>(index)]=static_cast<std::int16_t>(std::clamp(TapePcm::roundEven(value*gain*Constants::PcmScale),Constants::PcmMinimum,Constants::PcmScale));
    }
    return result;
}
}
