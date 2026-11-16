#pragma once
#include "Export.h"
#include "ScpHeaderConstants.h"
#include <cstdint>
#include <string>
namespace RetroMediaLab::MediaEngine::Scp {
struct Header {
 std::uint8_t version=0,diskType=0,revolutions=0,startTrack=0,endTrack=0,flags=0,bitCellEncoding=0,heads=0,resolution=0;
 std::uint32_t checksum=0;
 std::int32_t trackCount()const noexcept{return std::int32_t(endTrack)-startTrack+ScpHeaderConstants::InclusiveTrackCount;}
 std::int32_t resolutionNanoseconds()const noexcept{return ScpHeaderConstants::ResolutionStep*(std::int32_t(resolution)+ScpHeaderConstants::ResolutionIndexOffset);}
 RETROMEDIALAB_MEDIAENGINE_API std::string versionText()const;
};
}
