#include "Serial.h"
#include "Constants.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <cmath>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Fsk {
namespace {
using namespace Constants;
struct ToneResult {std::vector<std::uint8_t> tones;std::vector<std::pair<int,double>> recoveries;};
ToneResult toneLevels(std::span<const std::int32_t> samples,int rate,int zero,int one,std::stop_token token) {
    ToneResult out{std::vector<std::uint8_t>(samples.size(),1),{}};const auto zeroHalf=double(rate)/(HalfWavesPerCycle*double(zero)),oneHalf=double(rate)/(HalfWavesPerCycle*double(one));const auto minimum=std::min(zeroHalf,oneHalf)*MinimumHalfWaveRatio,maximum=std::max(zeroHalf,oneHalf)*MaximumHalfWaveRatio;
    int previous=0,edge=-1,carrierWaves=0;std::int64_t carrierSamples=0;
    const auto endCarrier=[&](int position){if(carrierWaves>=CarrierRecoveryHalfWaves)out.recoveries.emplace_back(position,carrierSamples/(carrierWaves*oneHalf));carrierWaves=0;carrierSamples=0;};
    for(int i=0;i<int(samples.size());++i){checkCancellation(token);const auto current=(samples[i]>0)-(samples[i]<0);if(!current||current==previous)continue;
        if(edge>=0){const auto duration=i-edge;if(duration>=minimum&&duration<=maximum){const bool mark=std::abs(duration-oneHalf)<std::abs(duration-zeroHalf);std::fill(out.tones.begin()+edge,out.tones.begin()+i,std::uint8_t(mark));if(mark){++carrierWaves;carrierSamples+=duration;}else endCarrier(edge);}else endCarrier(edge);}edge=i;previous=current;
    }
    return out;
}
void validate(int dataBits,int stops){if(dataBits<1||dataBits>MaximumDataBits||stops<=0||stops>MaximumItems-MaximumDataBits-1)throw MediaError(DiagnosticCode::FskInvalid);}
}
DecodeResult decode(std::span<const std::int32_t> samples,int rate,int baud,int zero,int one,int dataBits,int stops,int minimumSamples,std::stop_token token) {
    validate(dataBits,stops);if(rate<=0||baud<=0||zero<=0||one<=0||minimumSamples<=0||samples.size()>MaximumItems)throw MediaError(DiagnosticCode::FskInvalid);checkCancellation(token);
    double samplesPerBit=double(rate)/baud;if(samplesPerBit<minimumSamples)return {{},0,0};const auto tones=toneLevels(samples,rate,zero,one,token);const auto nominal=samplesPerBit;std::size_t recovery=0;std::vector<std::uint8_t> bytes;std::vector<int> positions;int tested=0;
    for(int start=1;start<int(tones.tones.size());++start){checkCancellation(token);if(tones.tones[start]||!tones.tones[start-1])continue;while(recovery<tones.recoveries.size()&&tones.recoveries[recovery].first<=start)samplesPerBit=nominal*tones.recoveries[recovery++].second;
        const auto last=TapePcm::roundEven(start+(dataBits+double(stops)+Half)*samplesPerBit);if(last>=double(tones.tones.size()))break;
        if(tones.tones[std::size_t(TapePcm::roundEven(start+samplesPerBit/HalfWavesPerCycle))])continue;++tested;bool validStop=true;
        for(int stop=0;stop<stops;++stop)validStop=validStop&&tones.tones[std::size_t(TapePcm::roundEven(start+(dataBits+OneAndHalf+stop)*samplesPerBit))];if(!validStop)continue;
        std::uint8_t value=0;for(int bit=0;bit<dataBits;++bit)if(tones.tones[std::size_t(TapePcm::roundEven(start+(bit+OneAndHalf)*samplesPerBit))])value|=std::uint8_t(1<<bit);
        bytes.push_back(value);positions.push_back(start);start=int(last);
    }
    const auto valid=int(positions.size());
    return {std::move(bytes),valid,tested,std::move(positions)};
}
DecodeResult decodeFrames(std::span<const std::uint8_t> bits,int dataBits,int stops,std::stop_token token) {
    validate(dataBits,stops);if(bits.size()>MaximumItems)throw MediaError(DiagnosticCode::FskInvalid);checkCancellation(token);const auto frame=1+dataBits+stops;std::vector<std::uint8_t> bytes;std::vector<int> positions;int tested=0,valid=0;
    for(std::size_t index=0;index+frame<=bits.size();){checkCancellation(token);if(bits[index]){++index;continue;}++tested;bool validStop=true;for(int stop=0;stop<stops;++stop)validStop=validStop&&bits[index+1+dataBits+stop];if(!validStop){++index;continue;}std::uint8_t value=0;for(int bit=0;bit<dataBits;++bit)if(bits[index+bit+1])value|=std::uint8_t(1<<bit);bytes.push_back(value);positions.push_back(int(index));++valid;index+=frame;}
    return {std::move(bytes),valid,tested,std::move(positions)};
}
}
