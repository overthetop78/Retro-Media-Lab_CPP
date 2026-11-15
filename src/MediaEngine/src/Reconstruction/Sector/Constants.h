#pragma once
#include <array>
#include "../../Formats/Apple/GeometryConstants.h"
#include <cstdint>
#include <limits>
#include <string_view>

namespace RetroMediaLab::MediaEngine::Reconstruction::Sector::Constants {
inline constexpr std::int32_t EightyCylinders = Apple::Geometry::EightyCylinders, SingleHead = Apple::Geometry::SingleHead, DoubleHeads = Apple::Geometry::DoubleHeads;
inline constexpr std::int32_t ProDosBlockBytes = Apple::Geometry::AppleII::ProDosBlockSize, AppleSectorBytes = Apple::Geometry::AppleII::SectorSize, SectorsPerProDosBlock = Apple::Geometry::AppleII::SectorsPerProDosBlock;
inline constexpr std::int32_t LisaCylinders = Apple::Geometry::LisaFileWare::CylinderCount, LisaLinearThreshold = Apple::Geometry::LisaFileWare::LinearTrackThreshold;

inline constexpr std::array<std::uint32_t,5> MacIndexTicks{6091371,5594406,5084746,4571429,4067797};
inline constexpr std::uint32_t AppleIICellTicks = 160, IwmCellTicks = 80, LisaTicksPerSector = 500000;
inline constexpr std::array<std::uint32_t,4> Commodore1541CellTicks{130,140,150,160}, Commodore900CellTicks{86,93,100,106};
inline constexpr std::int32_t AppleIIFormat = 0, AppleII80Format = 0x24, MacSingleFormat = 0x02, MacDoubleFormat = 0x22, LisaFormat = 0x12;
inline constexpr std::int32_t AtariLinearSectors = 1, AtariStandardSectors = 18, AtariEnhancedSectors = 26, FirstSector = 1;
inline constexpr std::int32_t Rx02BlockBytes = 512, Rx02SectorBytes = 256, Rx02SectorsPerBlock = 2;
inline constexpr std::size_t JfdTagBytes = 4, JfdTagDensityIndex = 3;
inline constexpr std::uint8_t JfdOrdinaryTrack = 255, JfdFmDensity = 1;
inline constexpr std::uint32_t JfdDensityDivisor = 160;
inline constexpr auto MinimumAddress = std::numeric_limits<std::int32_t>::min(), MaximumAddress = std::numeric_limits<std::int32_t>::max();
inline constexpr std::string_view TagPrefix = "tag", FormatAttribute = "format", SectorCountAttribute = "sectorsPerTrack", TracksPerSideAttribute = "tracksPerSide";
}

