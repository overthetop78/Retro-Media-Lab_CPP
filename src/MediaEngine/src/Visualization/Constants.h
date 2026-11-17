#pragma once
#include <cstddef>
#include <array>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Visualization::Constants {
inline constexpr double FluxMinimumQuality=0,FluxMaximumQuality=1;
inline constexpr std::size_t FluxArcSamples=720,FluxMedianDivisor=2;
inline constexpr double FluxShortRatio=.65,FluxLongRatio=1.8,FluxMinimumSweep=.08,FluxStructureMinimumSweep=.18,FluxCircle=360,FluxTop=90;
inline constexpr double NanosecondsPerMillisecond=1000000,NanosecondsPerMinute=60000000000,FluxDurationTolerance=.08,FluxCountTolerance=.18,FluxDurationWeight=.65,FluxCountWeight=.35;
inline constexpr std::array FluxStructureKeys{"Visual.StructureKind.Sync","Visual.StructureKind.IdAddressMark","Visual.StructureKind.DataAddressMark","Visual.StructureKind.DeletedDataAddressMark","Visual.StructureKind.AmigaSync","Visual.StructureKind.AppleAddress","Visual.StructureKind.AppleData","Visual.StructureKind.CommodoreSync","Visual.StructureKind.CommodoreHeader","Visual.StructureKind.FormatHeader","Visual.StructureKind.FormatData","Visual.StructureKind.TimingAnomaly"};
inline constexpr double MinimumSequentialLength=1;
inline constexpr std::size_t PreviewBytes=4096;
inline constexpr std::size_t ReadBufferSize=64*1024;
inline constexpr std::size_t LaneFace=0,LaneTrack=1,LaneChannel=2;
inline constexpr char FirstDigit='0',LastDigit='9';
inline constexpr std::string_view Position="position",Surface="surface",Length="length",Session="sessionNumber",Track="trackNumber",Layer="layerNumber",Face="faceNumber",Channel="channelNumber",Surfaces="surfaces",Layers="layers",Providers="providers",BlockImage="blocks",Descriptor="descriptor";
}
