#pragma once
#include <string_view>
#include <cstddef>
namespace RetroMediaLab::MediaFileSystems::AtariMetadata::BootConstants {
inline constexpr std::size_t HeaderLength=6,CountOffset=1,LoadOffset=2,SectorSize=128,AddressSpace=65536;
inline constexpr std::string_view Id="atari-boot-disk",Display="Atari boot disk",Bootable="bootable",Direct="direct-boot-program";
}
