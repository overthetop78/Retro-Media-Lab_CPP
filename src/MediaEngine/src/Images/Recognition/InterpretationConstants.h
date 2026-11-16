#pragma once
#include "../../Formats/FormatIds.h"
#include <array>
namespace RetroMediaLab::MediaEngine::InterpretationConstants {
using namespace Formats;
inline constexpr std::array IsoFormats{UcsdIbmMfm,Commodore900Coherent,EpsonQx10_396,EpsonQx10_399,EpsonQx10Logo};
inline constexpr std::array DfsFormats{AcornDfsSingleSided,AcornDfsSingleSided80,AcornDfsDoubleSided,AcornDfsDoubleSided80,EpsonQx10_320};
inline constexpr std::array AdfsFormats{EpsonQx10_400};
inline constexpr std::uint32_t IsoBlockSize=512,DfsBlockSize=256,AdfsBlockSize=1024,MacMfmSectorCount=2880;
}
