#pragma once
#include <string_view>
#include <cstdint>
#include <cstddef>
namespace RetroMediaLab::MediaEngine::AmstradTape::Constants {
inline constexpr std::string_view DecoderId="amstrad-cpc-tape";
inline constexpr std::string_view EncoderId="amstrad-cpc-tape";
inline constexpr std::uint8_t HeaderSync=0x2c;
inline constexpr std::uint8_t DataSync=0x16;
inline constexpr int HeaderLength=64;
inline constexpr int SegmentDataLength=256;
inline constexpr int SegmentCrcLength=2;
inline constexpr int TrailerByteCount=8;
inline constexpr std::uint8_t TrailerByte=0xff;
inline constexpr int FileNameOffset=0;
inline constexpr int FileNameLength=16;
inline constexpr int BlockNumberOffset=16;
inline constexpr int LastBlockOffset=17;
inline constexpr int FileTypeOffset=18;
inline constexpr int DataLengthOffset=19;
inline constexpr int DataLocationOffset=21;
inline constexpr int FirstBlockOffset=23;
inline constexpr int LogicalLengthOffset=24;
inline constexpr int EntryAddressOffset=26;
inline constexpr std::uint16_t CrcPolynomial=0x1021;
inline constexpr std::uint16_t CrcSeed=0xffff;
inline constexpr std::uint16_t OnePulseTStates=1162;
inline constexpr std::uint16_t ZeroPulseTStates=581;
inline constexpr std::uint16_t PilotPulseCount=4096;
inline constexpr std::uint16_t PauseMilliseconds=10;
inline constexpr std::uint8_t UsedBitsInLastByte=8;
inline constexpr int PcmBitsPerSample=16;
inline constexpr std::int16_t PcmAmplitude=24576;
inline constexpr int MinimumPilotPulseCount=2048;
inline constexpr double PulseToleranceRatio=0.35;
inline constexpr int MaximumDataLength=2048,MaximumSegments=8,AlternateTrailerLength=4,DefaultSampleRate=44100,MaximumByte=255,ByteBits=8,WordSize=2,LastBit=7,PulsePair=2,NameDigits=3,DefaultFileType=2,DecimalRadix=10;
inline constexpr char DigitZero='0',Null='\0',Space=' ';
inline constexpr std::string_view MachineId="amstrad",WaveFormat="tape.wav",VocFormat="tape.voc",CswFormat="tape.csw",RawFormat="tape.raw",AudioFormat="tape.audio",FileNameKey="fileName",BlockNumberKey="blockNumber",LastBlockKey="lastBlock",FirstBlockKey="firstBlock",FileTypeKey="fileType",DataLengthKey="dataLength",DataLocationKey="dataLocation",LogicalLengthKey="logicalLength",EntryAddressKey="entryAddress",SourceKindKey="sourceKind",SourceKind="amstrad-cpc-tape-records",HeaderIntegrityKey="headerIntegrityValid",DataIntegrityKey="dataIntegrityValid",ReconstructionKey="reconstructedSignal",TrueValue="True",FalseValue="False",DefaultNamePrefix="BLOCK",EncodedWave="encoded.wav",ChannelKey="channel",InterleavedKey="interleaved",ChannelZero="0";
inline constexpr std::size_t MaximumPcmSamples=128*1024*1024;
}
