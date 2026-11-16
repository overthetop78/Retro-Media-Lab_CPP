#pragma once
#include "Container.h"
namespace RetroMediaLab::MediaEngine::Vhd {
struct Geometry {std::uint16_t cylinders;std::uint8_t heads,sectors;};
Geometry calculateGeometry(std::int64_t sectors);
std::vector<std::uint8_t> createFooter(const BlockImage&,std::uint32_t diskType,std::uint64_t dataOffset);
std::vector<std::uint8_t> createDynamicHeader(std::uint32_t count);
}
