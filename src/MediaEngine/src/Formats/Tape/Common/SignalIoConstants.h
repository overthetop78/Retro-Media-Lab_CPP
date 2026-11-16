#pragma once
#include <RetroMediaLab/MediaEngine/TapeFormatIds.h>
#include <array>
#include <string_view>
namespace RetroMediaLab::MediaEngine::TapeSignal::IoConstants {
inline constexpr std::string_view WaveExtension=".wav",UefExtension=".uef",CswExtension=".csw",RawExtension=".raw",AtomExtension=".tap",SamplePositionKey="samplePosition",ByteOffsetKey="byteOffset";
inline constexpr std::array<std::string_view,4> Writable{WaveExtension,UefExtension,CswExtension,RawExtension};
inline constexpr double MaximumCarrierRate=6500,MinimumCarrierRate=3500;
inline constexpr int MinimumCarrierEdges=10,SerialBitsPerByte=10,DefaultBaud=1200,TicksPerSecond=10000000;
}
