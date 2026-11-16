#pragma once
#include <array>
#include <string_view>
namespace RetroMediaLab::MediaEngine::SequentialDecodingConstants {
inline constexpr std::string_view InternalNameKey="internalName";
inline constexpr std::array<std::string_view,8> SegmentNames{"Unknown","Samples","Pulse","Carrier","Silence","DataBlock","Record","TapeMark"};
}
