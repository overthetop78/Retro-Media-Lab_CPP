#include "Serial.h"
#include "Constants.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <cmath>
#include <bit>
namespace RetroMediaLab::MediaEngine::Fsk {
namespace {
using namespace Constants;
void appendBit(std::vector<std::int16_t>& samples,std::int16_t& level,int rate,int baud,int zero,int one,bool value,std::stop_token token) {
    if(rate<=0||baud<=0||zero<=0||one<=0||samples.size()>MaximumItems)throw MediaError(DiagnosticCode::FskInvalid);
    const auto frequency=value?one:zero;const auto halves=TapePcm::roundEven(frequency*double(HalfWavesPerCycle)/baud);const auto start=samples.size();const auto end=TapePcm::roundEven((TapePcm::roundEven(start*double(baud)/rate)+1)*rate/baud);
    if(halves>MaximumItems||!std::isfinite(end)||end<0||end>MaximumItems)throw MediaError(DiagnosticCode::FskInvalid);
    for(int half=0;half<int(halves);++half){checkCancellation(token);const auto next=start+TapePcm::roundEven((half+1)*(end-start)/halves);if(next>MaximumItems)throw MediaError(DiagnosticCode::FskInvalid);if(next>samples.size())samples.insert(samples.end(),std::size_t(next)-samples.size(),level);level=std::bit_cast<std::int16_t>(std::uint16_t(0-std::uint16_t(level)));}
}
}
void appendCarrier(std::vector<std::int16_t>& samples,std::int16_t& level,int rate,int baud,int zero,int one,double seconds,std::stop_token token) {
    if(!std::isfinite(seconds)||seconds<0)throw MediaError(DiagnosticCode::FskInvalid);const auto count=TapePcm::roundEven(seconds*baud);if(!std::isfinite(count)||count<0||count>MaximumItems)throw MediaError(DiagnosticCode::FskInvalid);for(int bit=0;bit<int(count);++bit)appendBit(samples,level,rate,baud,zero,one,true,token);
}
void appendFrame(std::vector<std::int16_t>& samples,std::int16_t& level,int rate,int baud,int zero,int one,int bits,int stops,std::uint8_t value,std::stop_token token) {
    if(bits<1||bits>MaximumDataBits||stops<=0)throw MediaError(DiagnosticCode::FskInvalid);appendBit(samples,level,rate,baud,zero,one,false,token);for(int bit=0;bit<bits;++bit)appendBit(samples,level,rate,baud,zero,one,(value&(1<<bit))!=0,token);for(int stop=0;stop<stops;++stop)appendBit(samples,level,rate,baud,zero,one,true,token);
}
}
