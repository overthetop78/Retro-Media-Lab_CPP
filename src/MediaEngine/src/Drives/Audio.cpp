#include "Mechanics.h"
#include "../Formats/Tape/Common/Pcm.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <cmath>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace C=MechanicalAudio::Constants;
namespace {
void advanceOffset(std::int64_t& offset,int count){if(offset>std::numeric_limits<std::int64_t>::max()-count)throw MediaError(DiagnosticCode::MechanicalRequest);offset+=count;}
}
std::int64_t RecordedDrive::frames(const Sound& sound,int rate){
    const auto count=std::ceil(bank_->duration(selectedProfile(),sound.operation,sound.distance,sound.direction,sound.cylinder)*rate);
    if(!std::isfinite(count)||count<0||count>=static_cast<double>(std::numeric_limits<std::int64_t>::max()))throw MediaError(DiagnosticCode::MechanicalRequest);
    return static_cast<std::int64_t>(count);
}
std::vector<std::int16_t> RecordedDrive::render(const Sound& sound,int rate,int count,std::int64_t offset){return bank_->render(device_,sound.operation,rate,count,offset,selectedProfile(),volume_,sound.distance,sound.direction,sound.cylinder);}
std::vector<std::int16_t> RecordedDrive::renderAudio(std::int32_t rate,std::int32_t count){
    if(rate<=0||count<0)throw MediaError(DiagnosticCode::MechanicalRequest);
    std::lock_guard lock(mutex_);
    if(!enabled_)return std::vector<std::int16_t>(static_cast<std::size_t>(count));
    if(device_==C::Tape){
        std::vector<std::int16_t> tape(static_cast<std::size_t>(count));int position=0;
        while(!events_.empty()&&position<count){
            const auto length=frames(events_.front(),rate);
            const auto remaining=static_cast<int>(std::min<std::int64_t>(count-position,std::max<std::int64_t>(0,length-eventOffset_)));
            const auto samples=render(events_.front(),rate,remaining,eventOffset_);std::copy(samples.begin(),samples.end(),tape.begin()+position);
            position+=remaining;advanceOffset(eventOffset_,remaining);
            if(eventOffset_>=length){events_.pop_front();eventOffset_=0;}
        }
        if(tapeOperation_!=C::None&&position<count){const auto samples=render({tapeOperation_},rate,count-position,tapeOffset_);std::copy(samples.begin(),samples.end(),tape.begin()+position);advanceOffset(tapeOffset_,count-position);}
        return tape;
    }
    std::vector<double> mixed(static_cast<std::size_t>(count));int destination=0;
    const auto mix=[&](const auto& samples){for(std::size_t i=0;i<samples.size();++i)mixed[static_cast<std::size_t>(destination)+i]+=samples[i];};
    while(destination<count){
        auto remaining=count-destination;auto playMotor=audibleMotor_;
        if(!events_.empty()){
            const auto& sound=events_.front();if(!eventOffset_&&sound.operation==C::Stop){audibleMotor_=false;playMotor=false;}
            const auto length=frames(sound,rate);remaining=static_cast<int>(std::min<std::int64_t>(remaining,std::max<std::int64_t>(0,length-eventOffset_)));
            if(remaining>0){mix(render(sound,rate,remaining,eventOffset_));advanceOffset(eventOffset_,remaining);}
            if(eventOffset_>=length){events_.pop_front();eventOffset_=0;}
        }
        if(!remaining)continue;
        if(playMotor){const auto operation=state_.writing?C::Write:state_.reading?C::Read:C::Motor;mix(render({std::string(operation)},rate,remaining,motorOffset_));advanceOffset(motorOffset_,remaining);}
        for(std::size_t i=voices_.size();i>0;){--i;auto& voice=voices_[i];const auto length=frames(voice.sound,rate);
            const auto size=static_cast<int>(std::min<std::int64_t>(remaining,std::max<std::int64_t>(0,length-voice.offset)));
            if(size>0)mix(render(voice.sound,rate,size,voice.offset));advanceOffset(voice.offset,remaining);
            if(voice.offset>=length)voices_.erase(voices_.begin()+static_cast<std::ptrdiff_t>(i));
        }
        destination+=remaining;
    }
    double peak=0;for(const auto value:mixed)peak=std::max(peak,std::abs(value));
    const auto gain=peak>C::PcmScale?C::PcmScale/peak:1;
    std::vector<std::int16_t> result(mixed.size());for(std::size_t i=0;i<result.size();++i)result[i]=static_cast<std::int16_t>(std::clamp(TapePcm::roundEven(mixed[i]*gain),C::PcmMinimum,C::PcmScale));
    return result;
}
}
