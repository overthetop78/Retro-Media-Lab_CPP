#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::I86f::Constants {
inline constexpr std::array<std::uint8_t,4> Signature{0x38,0x36,0x42,0x46};
inline constexpr std::size_t MinimumFileLength=8,FileFlagsOffset=6,FlagsBytes=2,TableOffset=8,TableEntryBytes=4,EntriesPerSide=256,TwoSideEntries=512;
inline constexpr std::size_t StandardHeaderBytes=6,ExtendedHeaderBytes=10,TrackFlagsOffset=0,ExplicitBitCountOffset=2,WordBits=16,BytesPerWord=2,BitsPerByte=8;
inline constexpr std::uint8_t MostSignificantBit=0x80;
inline constexpr std::uint32_t MaximumOffset=2147483647,MaximumFluxTicks=4294967295,TicksPerCell=40,TickNanoseconds=25;
inline constexpr std::uint16_t NoFlags=0,TwoSided=0x0008,SpeedShiftMask=0x0060,ExtraBitCellCount=0x0080,ReverseByteOrder=0x0800,SpeedupOrExplicitBitCount=0x1000;
inline constexpr std::uint16_t EncodingMask=0x0018,MfmEncoding=0x0008;
inline constexpr std::string_view FormatId="raw.86f",Extension=".86f",FlagsKey="86f.flags";
}
