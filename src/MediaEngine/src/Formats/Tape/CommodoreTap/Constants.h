#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include <limits>
namespace RetroMediaLab::MediaEngine::CommodoreTap::Constants {
inline constexpr std::string_view C64Signature="C64-TAPE-RAW";
inline constexpr std::string_view C16Signature="C16-TAPE-RAW";
inline constexpr int HeaderSize=20;
inline constexpr int SignatureOffset=0;
inline constexpr int SignatureLength=12;
inline constexpr int VersionOffset=12;
inline constexpr int PlatformOffset=13;
inline constexpr int VideoStandardOffset=14;
inline constexpr int ReservedOffset=15;
inline constexpr int DataLengthOffset=16;
inline constexpr int DataLengthSize=4;
inline constexpr std::uint8_t OriginalVersion=0;
inline constexpr std::uint8_t ExactPulseVersion=1;
inline constexpr std::uint8_t HalfWaveVersion=2;
inline constexpr int ShortPulseCycleMultiplier=8;
inline constexpr int ExtendedPulseByteCount=3;
inline constexpr int Version0OverflowCycles=255 * ShortPulseCycleMultiplier;
inline constexpr std::uint8_t C64Platform=0;
inline constexpr std::uint8_t Vic20Platform=1;
inline constexpr std::uint8_t C16Platform=2;
inline constexpr std::uint8_t PetPlatform=3;
inline constexpr std::uint8_t C5x0Platform=4;
inline constexpr std::uint8_t C6x0Platform=5;
inline constexpr std::uint8_t PalVideo=0;
inline constexpr std::uint8_t NtscVideo=1;
inline constexpr std::uint8_t OldNtscVideo=2;
inline constexpr std::uint8_t PalNVideo=3;
inline constexpr std::string_view PulseMetadataKey="commodoreTapPulse";
inline constexpr std::string_view SignatureMetadataKey="commodoreTapSignature";
inline constexpr std::string_view VersionMetadataKey="commodoreTapVersion";
inline constexpr std::string_view PlatformMetadataKey="commodoreTapPlatform";
inline constexpr std::string_view VideoStandardMetadataKey="commodoreTapVideoStandard";
inline constexpr std::string_view ClockRateMetadataKey="commodoreTapClockRate";
inline constexpr std::string_view CycleCountMetadataKey="commodoreTapCycleCount";
inline constexpr std::string_view HalfWaveMetadataKey="commodoreTapHalfWave";
inline constexpr std::string_view EncodedLengthMetadataKey="commodoreTapEncodedLength";
inline constexpr std::string_view DecoderId="commodore-tape";
inline constexpr std::string_view EncoderId="commodore-tape";
inline constexpr int DefaultC64PalClockRate=985248;
inline constexpr int ShortPulseCycles=0x30 * ShortPulseCycleMultiplier;
inline constexpr int MediumPulseCycles=0x42 * ShortPulseCycleMultiplier;
inline constexpr int LongPulseCycles=0x56 * ShortPulseCycleMultiplier;
inline constexpr double PulseToleranceRatio=0.25;
inline constexpr int DataBitsPerByte=8;
inline constexpr int PulsesPerBit=2;
inline constexpr int ParityBitsPerByte=1;
inline constexpr int PcmBitsPerSample=16;
inline constexpr int DefaultPcmSampleRate=44100;
inline constexpr std::int16_t PcmAmplitude=16000;
inline constexpr std::string_view SourceKindMetadataKey="sourceKind";
inline constexpr std::string_view ChecksumPresentMetadataKey="checksumPresent";
inline constexpr std::string_view ParityValidMetadataKey="parityValid";
struct Clock {std::uint8_t platform,video;int rate;};
inline constexpr std::array<Clock,14> TimerClockRates{{{C64Platform,PalVideo,985248},{C64Platform,NtscVideo,1022730},{C64Platform,OldNtscVideo,1022730},{C64Platform,PalNVideo,1023440},{Vic20Platform,PalVideo,1108405},{Vic20Platform,NtscVideo,1022727},{C16Platform,PalVideo,886724},{C16Platform,NtscVideo,894886},{PetPlatform,PalVideo,1000000},{PetPlatform,NtscVideo,1000000},{C5x0Platform,PalVideo,985248},{C5x0Platform,NtscVideo,1022730},{C6x0Platform,PalVideo,2000000},{C6x0Platform,NtscVideo,2000000}}};
inline constexpr std::string_view FormatId="tape.commodore-tap",WriterId="tape-commodore-tap",Extension=".tap",MachineId="commodore",WaveFormat="tape.wav",CswFormat="tape.csw",RawFormat="tape.raw",TrueValue="True",FalseValue="False",TapSource="tap-pulses",WaveSource="wav-pulses",ChannelKey="channel",InterleavedKey="interleaved",ChannelZero="0",EncodedWave="encoded.wav";
inline constexpr int ByteBits=8,WordShift=16,MaximumByte=255,MaximumCycles=0x00ffffff,BufferSize=65536,MaximumRecordSize=4,DecimalRadix=10,FramePulseCount=2+(DataBitsPerByte+ParityBitsPerByte)*PulsesPerBit;
inline constexpr std::size_t MaximumPcmSamples=128*1024*1024;
inline constexpr std::size_t MaximumPulseCount=std::numeric_limits<int>::max();
}
