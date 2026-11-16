#include "Signal.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::MfmEmu {
namespace {
std::uint64_t cellCount(std::size_t bytes,std::uint32_t rate,std::uint32_t period){using namespace Constants;if(!rate||!period||!bytes||bytes%WordLength)throw MediaError(DiagnosticCode::MfmSignal);if(rate>NanosecondsPerSecond)throw MediaError(DiagnosticCode::MfmSignalRate);return std::min(std::uint64_t(bytes)*BitsPerByte,(std::uint64_t(period)*rate+NanosecondsPerSecond-1)/NanosecondsPerSecond);}
std::uint64_t cellTime(std::uint64_t cell,std::uint32_t rate){return cell*Constants::NanosecondsPerSecond/rate;}
bool get(std::span<const std::uint8_t> bytes,std::uint64_t cell){using namespace Constants;return (qFromLittleEndian<std::uint32_t>(bytes.data()+cell/BitsPerWord*WordLength)&(FirstWordBit>>(cell%BitsPerWord)))!=0;}
void set(std::span<std::uint8_t> bytes,std::uint64_t cell,bool value){using namespace Constants;const auto offset=cell/BitsPerWord*WordLength;const auto word=qFromLittleEndian<std::uint32_t>(bytes.data()+offset),mask=FirstWordBit>>(cell%BitsPerWord);qToLittleEndian(value?word|mask:word&~mask,bytes.data()+offset);}
}
Flux::NanosecondRevolution readSignal(std::span<const std::uint8_t> bytes,std::uint32_t rate,std::uint32_t captureStart,std::uint32_t period,std::stop_token token){
    checkCancellation(token);const auto count=cellCount(bytes.size(),rate,period);const auto start=captureStart%period;std::vector<std::uint32_t> transitions;
    for(std::uint64_t cell=0;cell<count;++cell){checkCancellation(token);if(get(bytes,cell)){const auto phase=std::uint32_t((start+cellTime(cell,rate))%period);transitions.push_back(phase?phase:period);}}std::sort(transitions.begin(),transitions.end());Flux::NanosecondRevolution result{period,{}};std::uint32_t previous{};for(auto position:transitions){result.intervals.push_back(position-previous);previous=position;}return result;
}
std::vector<std::uint8_t> writeSignalWindow(std::span<const std::uint8_t> bytes,std::uint32_t rate,std::uint32_t captureStart,std::uint32_t period,std::uint32_t start,std::uint32_t duration,std::span<const std::uint32_t> offsets,bool ending,std::stop_token token){
    using namespace Constants;checkCancellation(token);const auto count=cellCount(bytes.size(),rate,period);std::optional<std::uint32_t> previous;for(auto offset:offsets){if(offset>duration||(!ending&&offset==duration)||(previous&&offset<=*previous))throw MediaError(DiagnosticCode::MfmWindow);previous=offset;}std::vector<std::uint8_t> result(bytes.begin(),bytes.end());if(!duration)return result;start%=period;captureStart%=period;
    for(std::uint64_t cell=0;cell<count;++cell){checkCancellation(token);const auto phase=(captureStart+cellTime(cell,rate))%period,distance=(phase+period-start)%period;if(distance<duration||(ending&&distance==duration))set(result,cell,false);}
    for(auto offset:offsets){checkCancellation(token);const auto next=std::uint64_t(offset)+period;if(next<duration||(ending&&next==duration))continue;const auto phase=(std::uint64_t(start)+offset)%period,stored=(phase+period-captureStart)%period;auto cell=(stored*rate+NanosecondsPerSecond-1)/NanosecondsPerSecond;const auto full=(std::uint64_t(period)*rate+NanosecondsPerSecond-1)/NanosecondsPerSecond;if(cell==full)cell=0;if(cell>=count)throw MediaError(DiagnosticCode::MfmUncapturedWrite);set(result,cell,true);}
    return result;
}
}
