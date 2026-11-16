#include "PulseIntervals.h"
#include "PcmConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::TapePcm {
void pulseIntervals(std::span<const std::int32_t> samples,int rate,const PulseConsumer& consume,std::stop_token token){
    checkCancellation(token);if(rate<=0||!consume)throw MediaError(DiagnosticCode::TapePcmProfile);if(samples.empty())return;
    bool sign=samples.front()>=0;std::size_t previous=0;const auto threshold=std::max<std::int64_t>(1,(std::int64_t(rate)+Constants::SilenceDivisor-1)/Constants::SilenceDivisor);
    for(std::size_t index=0;index<samples.size();++index){
        checkCancellation(token);
        if(samples[index]==0&&(index==0||samples[index-1]!=0)){
            auto end=index;while(end<samples.size()&&samples[end]==0){checkCancellation(token);++end;}
            if(end-index>=static_cast<std::uint64_t>(threshold)){
                if(index>previous)consume({index-previous,false});consume({end-index,true});previous=end;if(end==samples.size())return;sign=samples[end]>=0;index=end;continue;
            }
        }
        const bool next=samples[index]>=0;if(next==sign)continue;if(index>previous)consume({index-previous,false});previous=index;sign=next;
    }
    if(previous<samples.size())consume({samples.size()-previous,false});
}
}
