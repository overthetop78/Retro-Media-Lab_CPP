#pragma once
#include <RetroMediaLab/MediaFileSystems/SectorReader.h>
#include "Dos/Constants.h"
namespace RetroMediaLab::MediaFileSystems::AtariMetadata {
inline constexpr auto& Formats=AtariDos::Constants::Formats;
bool supported(std::string_view);
bool kFile(const SectorImage&,std::stop_token);
FileSystemVolume readKFile(const SectorImage&,std::stop_token);
bool clk(const SectorImage&,std::stop_token);
FileSystemVolume readClk(const SectorImage&,std::stop_token);
bool bootDisk(const SectorImage&,std::stop_token);
FileSystemVolume readBootDisk(const SectorImage&,std::stop_token);
}
