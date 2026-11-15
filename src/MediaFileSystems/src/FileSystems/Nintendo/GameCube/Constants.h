#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::GameCubeFstConstants {
inline constexpr std::string_view SourceExtension=".gcm", IsoExtension=".iso", SourceExtensionKey="sourceExtension", DisplayName="Nintendo GameCube FST", FileAttribute="FST", FileEntryType="nintendo-gamecube-file", DirectoryEntryType="nintendo-gamecube-directory", OffsetMetadata="offset", LengthMetadata="length", DefaultNamePrefix="entry-";
inline constexpr int DiscHeaderLength=0x430, DiscMagicOffset=0x1c, FstOffsetField=0x424, FstSizeField=0x428, FstEntrySize=12, LocationField=4, LengthField=8, SectorSize=2048, MaximumEntryCount=1000000, MaximumDirectoryDepth=256, MaximumFileContentLength=67108864, NameDigits=6;
inline constexpr std::uint32_t DiscMagic=0xc2339f3d, DirectoryFlag=0x80000000, NameOffsetMask=0x00ffffff;
inline constexpr char Zero='0';
}
