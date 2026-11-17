#pragma once
#include <cstddef>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Visualization::Constants {
inline constexpr std::size_t PreviewBytes=4096;
inline constexpr std::size_t ReadBufferSize=64*1024;
inline constexpr std::size_t LaneFace=0,LaneTrack=1,LaneChannel=2;
inline constexpr char FirstDigit='0',LastDigit='9';
inline constexpr std::string_view Position="position",Surface="surface",Length="length",Session="sessionNumber",Track="trackNumber",Layer="layerNumber",Face="faceNumber",Channel="channelNumber",Surfaces="surfaces",Layers="layers",Providers="providers",BlockImage="blocks",Descriptor="descriptor";
}
