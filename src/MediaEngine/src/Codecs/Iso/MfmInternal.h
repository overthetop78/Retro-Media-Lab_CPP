#pragma once
#include "Mfm.h"
namespace RetroMediaLab::MediaEngine::IsoMfm::Internal {
bool physical(const Encoding::Request&);
bool complete(const Encoding::Request&);
void validateAddress(std::string_view,std::int32_t);
std::uint8_t sizeCode(const Encoding::Sector&);
void run(Flux::Bits&,std::uint8_t,std::int32_t,std::stop_token);
void address(Flux::Bits&,const Encoding::Request&,const Encoding::Sector&,std::uint8_t,std::int32_t gap,std::stop_token);
void data(Flux::Bits&,const Encoding::Sector&,std::int32_t gap,std::stop_token);
}
