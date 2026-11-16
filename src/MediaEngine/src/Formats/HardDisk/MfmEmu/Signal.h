#pragma once
#include "../../../Models/Flux/Conversion.h"
namespace RetroMediaLab::MediaEngine::MfmEmu {
RETROMEDIALAB_MEDIAENGINE_API Flux::NanosecondRevolution readSignal(std::span<const std::uint8_t>,std::uint32_t rate,std::uint32_t captureStart,std::uint32_t period,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> writeSignalWindow(std::span<const std::uint8_t>,std::uint32_t rate,std::uint32_t captureStart,std::uint32_t period,std::uint32_t start,std::uint32_t duration,std::span<const std::uint32_t> offsets,bool includeEndingInstant=false,std::stop_token={});
}
