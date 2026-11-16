#pragma once
#include <string_view>
namespace RetroMediaLab::MediaEngine::SequentialConversionConstants {
inline constexpr std::string_view SourcePrefix="source.",ReconstructedKey="reconstructedSignal",TrueValue="true",MonoValue="1",AtomFormat="tape.atom-tap",UefFormat="tape.uef",WaveFormat="tape.wav",CswFormat="tape.csw",RawFormat="tape.raw",RawExtension=".raw";
inline constexpr int SignalAmplitude=16000;
}
