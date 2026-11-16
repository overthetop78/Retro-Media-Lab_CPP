#pragma once
#include "../../Common/Chd/ContainerConstants.h"
#include <array>
namespace RetroMediaLab::MediaEngine::ChdHardDisk::Constants {
using namespace Chd::Constants;
inline constexpr std::string_view FormatId="hdd.chd",Extension=".chd",WriterId="hard-disk-chd",VersionKey="version",VariantKey="variant",HunkSizeKey="hunkSize",FixedVariant="Fixed";
inline constexpr std::array<std::string_view,4> GeometryPrefixes{"CYLS:","HEADS:","SECS:","BPS:"};
inline constexpr char GeometrySeparator=',';
inline constexpr std::uint32_t MetadataTag=0x47444444,DefaultHunkBytes=512*1024;
inline constexpr std::uint64_t MaximumSigned=0x7fffffffffffffff,MaximumField=0xffffffff,MaximumBuffer=0x7fffffff;
inline constexpr int GeometryFields=4,Sha1Offset=64,Sha1Bytes=20,GeometryHeads=1,GeometrySectors=2,GeometryBytes=3;
}
