#include "Pulses.h"
#include "../../../Formats/Tape/Common/PulseIntervals.h"
#include "../../../Formats/Tape/SpectrumTap/Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <cmath>
#include <limits>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::SpectrumTape {
namespace {
using namespace SpectrumTap::Constants;
bool close(double actual,double expected){return std::abs(actual-expected)<=expected*PulseToleranceRatio;}
}
std::vector<std::vector<std::uint8_t>> detect(std::span<const std::int32_t> samples,int rate,std::stop_token token){
    checkCancellation(token);if(rate<=0)throw MediaError(DiagnosticCode::SpectrumTapePcm);
    std::vector<double> pulses;TapePcm::pulseIntervals(samples,rate,[&](auto interval){pulses.push_back(interval.silence?std::numeric_limits<double>::infinity():interval.samples*double(Tzx::Constants::TStatesPerSecond)/rate);},token);
    std::vector<std::vector<std::uint8_t>> blocks;
    for(std::size_t index=0;index<pulses.size();){
        checkCancellation(token);const auto start=index;
        while(index<pulses.size()&&close(pulses[index],PilotPulseTStates)){checkCancellation(token);++index;}
        if(index-start<MinimumDetectedPilotPulseCount||index+1>=pulses.size()||!close(pulses[index],SyncFirstPulseTStates)||!close(pulses[index+1],SyncSecondPulseTStates)){
            if(index==start)++index;continue;
        }
        index+=PulsePair;std::vector<std::uint8_t> bytes;int bit=0;std::uint8_t byte{};
        while(index+1<pulses.size()){
            checkCancellation(token);const bool zero=close(pulses[index],ZeroPulseTStates)&&close(pulses[index+1],ZeroPulseTStates);
            const bool one=close(pulses[index],OnePulseTStates)&&close(pulses[index+1],OnePulseTStates);if(!zero&&!one)break;
            if(one)byte|=std::uint8_t(MostSignificantBit>>bit);index+=PulsePair;
            if(++bit==ByteBits){bytes.push_back(byte);byte=0;bit=0;}
        }
        if(bytes.size()>=MinimumBlockLength)blocks.push_back(std::move(bytes));
    }
    return blocks;
}
}
