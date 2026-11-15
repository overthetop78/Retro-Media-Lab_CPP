#pragma once
#include "Crc16Constants.h"
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine::Crc16 {
RETROMEDIALAB_MEDIAENGINE_API std::uint16_t update(std::uint16_t,std::uint8_t,std::uint16_t polynomial=Constants::Ccitt) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::uint16_t compute(MediaFileSystems::Bytes,std::uint16_t polynomial=Constants::Ccitt,std::uint16_t initial=Constants::AllBitsSet,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> append(MediaFileSystems::Bytes,std::uint16_t polynomial=Constants::Ccitt,std::uint16_t initial=Constants::AllBitsSet,std::stop_token={});
}
