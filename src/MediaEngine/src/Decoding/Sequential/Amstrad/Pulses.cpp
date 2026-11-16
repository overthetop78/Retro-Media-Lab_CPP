#include "Pulses.h"
#include "../../../Formats/Tape/Amstrad/Constants.h"
#include "../../../Formats/Tape/Tzx/Constants.h"
#include "../../../Formats/Tape/Common/PulseIntervals.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <cmath>
#include <limits>
namespace RetroMediaLab::MediaEngine::AmstradTape {
namespace {
using namespace Constants;
bool close(double actual,double expected){return std::isfinite(actual)&&std::isfinite(expected)&&expected>0&&std::abs(actual-expected)<=expected*PulseToleranceRatio;}
}
std::vector<std::vector<std::uint8_t>> detect(std::span<const std::int32_t> samples,int rate,std::stop_token token){
    std::vector<double> pulses;TapePcm::pulseIntervals(samples,rate,[&](auto value){pulses.push_back(value.silence?std::numeric_limits<double>::infinity():value.samples*double(Tzx::Constants::TStatesPerSecond)/rate);},token);
    std::vector<std::vector<std::uint8_t>> records;
    for(std::size_t index=0;index<pulses.size();){
        checkCancellation(token);const auto start=index;const auto pilot=pulses[index];while(index<pulses.size()&&close(pulses[index],pilot)){checkCancellation(token);++index;}
        if(index-start<MinimumPilotPulseCount||index+1>=pulses.size()||!close(pulses[index],pilot/PulsePair)||!close(pulses[index+1],pilot/PulsePair)){index=start+1;continue;}
        index+=PulsePair;std::vector<std::uint8_t> bytes;int bit=0;std::uint8_t byte{};
        while(index+1<pulses.size()){
            checkCancellation(token);const auto first=pulses[index],second=pulses[index+1];if(!close(first,second))break;
            if(close(first,pilot/PulsePair)){}else if(close(first,pilot))byte|=std::uint8_t(1<<(LastBit-bit));else break;
            index+=PulsePair;if(++bit==ByteBits){bytes.push_back(byte);byte=0;bit=0;}
        }
        if(!bytes.empty())records.push_back(std::move(bytes));
    }
    return records;
}
}
