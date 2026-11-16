#pragma once
#include "../../../Sources/AtomicImageWriter.h"
namespace RetroMediaLab::MediaEngine::Csw {
void inflatePulses(std::span<const std::uint8_t>,const ImageOutput&,std::stop_token);
void deflatePulses(std::span<const std::uint8_t>,const ImageOutput&,std::stop_token);
}
