#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Nib::Constants {
inline constexpr std::size_t TrackLengthBytes=6656,BitsPerByte=8,MaximumTrackBitCount=TrackLengthBytes*BitsPerByte;
inline constexpr std::uint8_t TrackFillByte=0xff;
inline constexpr std::int32_t MinimumCredibleRwts18TrackCount=2,RwtsSectorCount=6,RwtsSectorSize=768;
inline constexpr std::string_view AppleIIGcr="apple2.gcr";
}
