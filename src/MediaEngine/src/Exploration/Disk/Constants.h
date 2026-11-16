#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::DiskExplorationConstants {
inline constexpr std::string_view UnknownFormat="unknown";
inline constexpr std::uint32_t UnknownBlockSize=1,UnknownDimension=1;
inline constexpr std::uint64_t UnknownBlockCount=1;
}
