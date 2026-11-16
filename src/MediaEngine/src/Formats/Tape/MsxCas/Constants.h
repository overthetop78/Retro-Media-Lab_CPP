#pragma once
#include <string_view>
#include <array>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::MsxCas::Constants {
inline constexpr std::array<std::uint8_t,8> Separator{0x1f,0xa6,0xde,0xba,0xcc,0x13,0x7d,0x74};
inline constexpr int SeparatorLength=8;
inline constexpr int FileTypeMarkerLength=10;
inline constexpr int FileNameLength=6;
inline constexpr std::uint8_t BinaryFileMarker=0xd0;
inline constexpr std::uint8_t TokenizedBasicFileMarker=0xd3;
inline constexpr std::uint8_t AsciiFileMarker=0xea;
inline constexpr std::string_view SeparatorMetadataKey="msxCasSeparator";
inline constexpr std::string_view FileTypeMetadataKey="msxCasFileType";
inline constexpr std::string_view TimingMetadataKey="msxCasTiming";
inline constexpr std::string_view DecoderId="msx-tape";
inline constexpr std::string_view EncoderId="msx-tape";
inline constexpr int StandardBaudRate=1200;
inline constexpr int ZeroFrequency=1200;
inline constexpr int OneFrequency=2400;
inline constexpr int DataBitsPerFrame=8;
inline constexpr int StopBitsPerFrame=2;
inline constexpr int SerialFrameBitCount=1 + DataBitsPerFrame + StopBitsPerFrame;
inline constexpr int MinimumSamplesPerBit=8;
inline constexpr int PcmBitsPerSample=16;
inline constexpr int DefaultPcmSampleRate=44100;
inline constexpr std::int16_t PcmAmplitude=16000;
inline constexpr double LongSilenceSeconds=2;
inline constexpr double ShortSilenceSeconds=1;
inline constexpr double LongCarrierSeconds=2;
inline constexpr double ShortCarrierSeconds=0.5;
inline constexpr std::string_view SourceKindMetadataKey="sourceKind";
inline constexpr std::string_view ReconstructionMetadataKey="reconstructedSignal";
inline constexpr std::string_view SerialFramesValidMetadataKey="serialFramesValid";
inline constexpr std::string_view FormatId="tape.msx-cas",Extension=".cas",WriterId="tape-msx-cas",MachineId="msx",WaveFormat="tape.wav",NoTiming="not-stored",BinaryType="binary",BasicType="basic",AsciiType="ascii",TrueValue="True",ChannelKey="channel",InterleavedKey="interleaved",ChannelZero="0",EncodedWave="encoded.wav";
inline constexpr int BufferSize=65536;
inline constexpr std::size_t MaximumPcmSamples=128*1024*1024;
}
