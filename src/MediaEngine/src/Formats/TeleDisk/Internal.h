#pragma once
#include "Records.h"
namespace RetroMediaLab::MediaEngine::TeleDisk {
std::uint8_t checkedByte(std::int64_t);
Header createHeader(const MediaFileSystems::SectorImage&,std::stop_token);
std::shared_ptr<const MediaFileSystems::SectorImage> buildSectorImage(const std::vector<Track>&,std::uint8_t,std::stop_token);
}
