#include "Fm.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::DataGeneral {
namespace {std::uint16_t update(std::uint16_t value,std::uint8_t input){return std::uint16_t(((value&Constants::ByteMask)^(value>>Constants::BitsPerByte))|(((value&Constants::ByteMask)^input)<<Constants::BitsPerByte));}}
std::uint16_t checksum(MediaFileSystems::Bytes data,std::stop_token token){checkCancellation(token);std::uint16_t value=0;for(auto input:data){checkCancellation(token);value=update(value,input);}return update(value,Constants::TerminalByte);}
}
