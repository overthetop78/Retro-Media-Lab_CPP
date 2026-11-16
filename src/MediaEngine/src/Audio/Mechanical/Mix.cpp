#include <RetroMediaLab/MediaEngine/MechanicalSound.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "Constants.h"
#include "../../Formats/Tape/Common/Pcm.h"
#include <algorithm>
#include <cmath>
namespace RetroMediaLab::MediaEngine {
std::vector<std::int16_t> protectMechanicalAudioMix(std::span<const double> samples){
    namespace C=MechanicalAudio::Constants;double peak=0;for(auto value:samples){if(!std::isfinite(value))throw MediaError(DiagnosticCode::AudioPcmData);peak=std::max(peak,std::abs(value));}
    const auto gain=peak>C::PcmScale?C::PcmScale/peak:1;std::vector<std::int16_t> result(samples.size());
    for(std::size_t i=0;i<samples.size();++i)result[i]=static_cast<std::int16_t>(std::clamp(TapePcm::roundEven(samples[i]*gain),C::PcmMinimum,C::PcmScale));return result;
}
}
