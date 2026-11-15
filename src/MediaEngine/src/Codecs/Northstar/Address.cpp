#include "Mfm.h"
namespace RetroMediaLab::MediaEngine::Northstar {
std::uint8_t pack(std::int32_t cylinder,std::int32_t sector)noexcept{return std::uint8_t((std::uint32_t(cylinder)&Constants::CylinderMask)<<Constants::CylinderShift|(std::uint32_t(sector)&Constants::SectorMask));}
Address unpack(std::uint8_t value)noexcept{return {std::uint8_t((value>>Constants::CylinderShift)&Constants::CylinderMask),std::uint8_t(value&Constants::SectorMask)};}
}
