#include "Protocol.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <cmath>
#include <set>
namespace RetroMediaLab::MediaEngine::CommodoreTape {
namespace {
using namespace CommodoreTap::Constants;
bool close(int actual,int expected){return std::abs(double(actual)-expected)<=expected*PulseToleranceRatio;}
bool bit(int first,int second,bool& value){
    if(close(first,ShortPulseCycles)&&close(second,MediumPulseCycles)){value=false;return true;}
    if(close(first,MediumPulseCycles)&&close(second,ShortPulseCycles)){value=true;return true;}return false;
}
bool byte(std::span<const Pulse> pulses,std::size_t& offset,std::uint8_t& value,bool& parity){
    if(offset>pulses.size()||pulses.size()-offset<FramePulseCount||!close(pulses[offset].cycles,LongPulseCycles)||!close(pulses[offset+1].cycles,MediumPulseCycles))return false;
    auto cursor=offset+PulsesPerBit;int ones=0;value=0;
    for(int index=0;index<DataBitsPerByte;++index){bool set{};if(!bit(pulses[cursor].cycles,pulses[cursor+1].cycles,set))return false;if(set){value|=std::uint8_t(1<<index);++ones;}cursor+=PulsesPerBit;}
    bool set{};if(!bit(pulses[cursor].cycles,pulses[cursor+1].cycles,set))return false;parity=((ones+int(set))&1)==1;offset=cursor+PulsesPerBit;return true;
}
}
std::vector<FrameBlock> decode(std::span<const Pulse> pulses,std::stop_token token){
    std::vector<FrameBlock> blocks;
    for(std::size_t scan=0;scan+1<pulses.size();){
        checkCancellation(token);if(!close(pulses[scan].cycles,LongPulseCycles)||!close(pulses[scan+1].cycles,MediumPulseCycles)){++scan;continue;}
        const auto start=scan;FrameBlock block;block.parityValid=true;std::uint8_t value{};bool parity{};
        while(byte(pulses,scan,value,parity)){checkCancellation(token);block.data.push_back(value);block.parityValid=block.parityValid&&parity;}
        if(block.data.empty()){scan=start+1;continue;}
        std::set<std::int64_t> seen;for(auto at=start;at<scan;++at)if(seen.insert(pulses[at].source).second)block.sources.push_back(pulses[at].source);blocks.push_back(std::move(block));
    }
    return blocks;
}
std::vector<int> encode(std::span<const std::uint8_t> bytes,std::stop_token token){
    if(bytes.size()>MaximumPulseCount/FramePulseCount)throw MediaError(DiagnosticCode::CommodoreTapeLimit);std::vector<int> pulses;pulses.reserve(bytes.size()*FramePulseCount);
    const auto addBit=[&](bool value){pulses.push_back(value?MediumPulseCycles:ShortPulseCycles);pulses.push_back(value?ShortPulseCycles:MediumPulseCycles);};
    for(auto value:bytes){checkCancellation(token);pulses.push_back(LongPulseCycles);pulses.push_back(MediumPulseCycles);int ones=0;for(int index=0;index<DataBitsPerByte;++index){const bool set=(value&(1<<index))!=0;addBit(set);ones+=set;}addBit((ones&1)==0);}
    return pulses;
}
}
