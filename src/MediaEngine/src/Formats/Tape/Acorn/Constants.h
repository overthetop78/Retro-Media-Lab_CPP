#pragma once
#include <string_view>
#include <cstddef>
namespace RetroMediaLab::MediaEngine::AcornTape::Constants {
inline constexpr std::string_view MachineId="acorn-bbc",WaveFormat="tape.wav",CswFormat="tape.csw",RawFormat="tape.raw",VocFormat="tape.voc",ImplicitSource="uef-implicit",WaveSource="wav-fsk",ByteOffsetKey="byteOffset",SamplePositionKey="samplePosition",ImplicitChunk="0100",ExplicitChunk="0102",DefinedChunk="0104",AtomChunk="0105",CyclesChunk="0114",CarrierChunk="0110",GapChunk="0116",ChannelKey="channel",InterleavedKey="interleaved",ChannelZero="0",EncodedWave="encoded.wav";
inline constexpr int FixedHeaderLength=17,LoadOffset=0,ExecuteOffset=4,NumberOffset=8,DataLengthOffset=10,FlagsOffset=12,NextOffset=13,RadixHex=16,RadixDecimal=10,AddressDigits=8,FlagDigits=2,ChunkDigits=4,SlowBaud=300,MaximumAscii=127;
inline constexpr std::size_t MaximumSignalSamples=128*1024*1024;
inline constexpr char ZeroDigit='0',AsciiReplacement='?';
}
