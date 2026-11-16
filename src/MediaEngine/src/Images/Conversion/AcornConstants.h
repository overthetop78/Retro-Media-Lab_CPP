#pragma once
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::AcornConversion::Constants {
inline constexpr std::string_view ContainerAdfId="sector-container-to-acorn-adf",RawAdfId="raw-to-acorn-adf",RawDfsId="raw-to-bbc-dfs";
inline constexpr std::uint32_t RawBlockSize=512,BbcBlockSize=256,ArchimedesBlockSize=1024,RawMapSize=512,DensityBase=160;
inline constexpr std::uint8_t BbcSizeCode=1,ArchimedesSizeCode=3,HighDensityRate=2,DoubleDensityRate=0,GoodDiagnostic=0;
inline constexpr std::size_t JfdMaximumTag=4;
}
