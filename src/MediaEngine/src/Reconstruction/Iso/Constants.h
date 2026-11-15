#pragma once
#include <cstdint>
namespace RetroMediaLab::MediaEngine::IsoReconstruction::Constants {
inline constexpr char DecoderKeySeparator='\0';
inline constexpr std::uint32_t ValidSectorScoreWeight=1000,DataSectorScoreWeight=10,InvalidSectorScorePenalty=1000;
inline constexpr std::uint64_t MaximumBlockCount=2147483647;
}
