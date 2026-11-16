#pragma once
#include <cstdint>
#include <cstddef>
#include <string_view>
namespace RetroMediaLab::MediaEngine::TapeSignalConstants {
inline constexpr int SampleRate=48000,MaximumSamples=128*1024*1024,FirstSignalChunk=0x100,FullPhase=360,HalfPhase=180,PairWidth=2,FloatWidth=4,ByteBits=8,SignalBits=16,ExplicitMaximum=15,SerialPrefix=3,SlowBaud=300,FastBaud=1200,SlowFactor=4,FastHalves=4,SlowHalves=2,SecurityHeader=5,CountBytes=3,SecurityFirstFlag=3,SecurityLastFlag=4,BitMask=7,MostSignificantBit=128,MaximumCycles=0xffffff,HighRate=4800,LowRate=2400,HexBase=16,ChunkIdDigits=4,NumberBuffer=128;
inline constexpr std::uint8_t DummyByte=0xaa;
inline constexpr char NoParity='N',EvenParity='E',OddParity='O',PartialCycle='P',WholeCycle='W',DigitZero='0';
}
