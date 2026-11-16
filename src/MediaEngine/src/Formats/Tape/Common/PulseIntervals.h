#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <span>
#include <cstdint>
#include <functional>
#include <stop_token>
namespace RetroMediaLab::MediaEngine::TapePcm {
struct PulseInterval {std::size_t samples;bool silence;};
using PulseConsumer=std::function<void(PulseInterval)>;
RETROMEDIALAB_MEDIAENGINE_API void pulseIntervals(std::span<const std::int32_t>,int sampleRate,const PulseConsumer&,std::stop_token={});
}
