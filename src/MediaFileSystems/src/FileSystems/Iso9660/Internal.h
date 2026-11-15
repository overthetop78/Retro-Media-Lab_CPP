#pragma once
#include "Constants.h"
#include "../Optical/Common.h"
namespace RetroMediaLab::MediaFileSystems::Iso {
enum class Mode {Primary,Joliet,RockRidge};
bool acceptDescriptor(Bytes,Mode);
bool acceptFileSystem(const OpticalTrack&,Bytes,Mode,std::stop_token);
std::string volumeName(Bytes,Mode);
std::string fileName(Bytes,Mode);
std::string entryName(Bytes,std::string,Mode);
std::vector<std::uint8_t> extent(const OpticalTrack&,std::uint32_t,std::uint32_t,std::stop_token);
std::pair<std::uint32_t,std::uint32_t> root(Bytes);
std::vector<std::uint8_t> descriptor(const ImageDocument&,const VolumeDescriptor&,Mode,std::stop_token);
FileSystemVolume read(const ImageDocument&,const VolumeDescriptor&,Mode,std::string_view,std::stop_token);
}
