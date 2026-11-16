#include "Internal.h"
namespace RetroMediaLab::MediaEngine::HxcStream {
void decodePulses(std::span<const std::uint8_t> bytes,std::uint32_t count,std::vector<std::uint32_t>& destination,std::stop_token token) {
    using namespace Constants;std::size_t at{};std::uint32_t decoded{};
    while(decoded<count&&at<bytes.size()){checkCancellation(token);const auto first=bytes[at++];std::uint32_t value=first;std::size_t tail{};
        if((first&PulseOneLimit)==0){}
        else if((first&PulseTwoMask)==PulseTwoPrefix){value=first&PulseTwoData;tail=TwoTailBytes;}
        else if((first&PulseThreeMask)==PulseThreePrefix){value=first&PulseThreeData;tail=ThreeTailBytes;}
        else if((first&PulseFourMask)==PulseFourPrefix){value=first&PulseFourData;tail=FourTailBytes;}
        else throw MediaError(DiagnosticCode::HxcPulseCode);
        if(tail>bytes.size()-at)throw MediaError(DiagnosticCode::HxcPulseTruncated);
        for(std::size_t i=0;i<tail;++i)value=(value<<ByteBits)|bytes[at++];if(value!=0)destination.push_back(value);++decoded;
    }
    if(decoded!=count)throw MediaError(DiagnosticCode::HxcPulseCount);
}
}
