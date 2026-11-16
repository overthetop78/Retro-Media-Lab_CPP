#pragma once
#include <string_view>
#include <array>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::Tzx::Constants {
inline constexpr std::string_view Signature="ZXTape!";
inline constexpr std::uint8_t EndOfTextMarker=0x1a;
inline constexpr std::uint8_t MajorVersion=1;
inline constexpr std::uint8_t MinorVersion=20;
inline constexpr int HeaderSize=10;
inline constexpr std::uint8_t StandardSpeedData=0x10;
inline constexpr std::uint8_t TurboSpeedData=0x11;
inline constexpr std::uint8_t PureTone=0x12;
inline constexpr std::uint8_t PulseSequence=0x13;
inline constexpr std::uint8_t PureData=0x14;
inline constexpr std::uint8_t DirectRecording=0x15;
inline constexpr std::uint8_t CswRecording=0x18;
inline constexpr std::uint8_t GeneralizedData=0x19;
inline constexpr std::uint8_t Pause=0x20;
inline constexpr std::uint8_t GroupStart=0x21;
inline constexpr std::uint8_t GroupEnd=0x22;
inline constexpr std::uint8_t Jump=0x23;
inline constexpr std::uint8_t LoopStart=0x24;
inline constexpr std::uint8_t LoopEnd=0x25;
inline constexpr std::uint8_t CallSequence=0x26;
inline constexpr std::uint8_t Return=0x27;
inline constexpr std::uint8_t Select=0x28;
inline constexpr std::uint8_t StopIf48K=0x2a;
inline constexpr std::uint8_t SetSignalLevel=0x2b;
inline constexpr std::uint8_t TextDescription=0x30;
inline constexpr std::uint8_t Message=0x31;
inline constexpr std::uint8_t ArchiveInfo=0x32;
inline constexpr std::uint8_t HardwareType=0x33;
inline constexpr std::uint8_t CustomInfo=0x35;
inline constexpr std::uint8_t Glue=0x5a;
inline constexpr int TStatesPerSecond=3500000;
inline constexpr std::uint16_t StandardPilotPulseTStates=2168;
inline constexpr std::uint16_t StandardSyncFirstPulseTStates=667;
inline constexpr std::uint16_t StandardSyncSecondPulseTStates=735;
inline constexpr std::uint16_t StandardZeroPulseTStates=855;
inline constexpr std::uint16_t StandardOnePulseTStates=1710;
inline constexpr std::uint16_t StandardHeaderPilotPulseCount=8063;
inline constexpr std::uint16_t StandardDataPilotPulseCount=3223;
inline constexpr std::uint8_t FullLastByteBitCount=8;
inline constexpr std::uint16_t StandardPauseMilliseconds=1000;
inline constexpr std::string_view BlockIdMetadataKey="tzxBlockId";
inline constexpr std::string_view DecoderId="tzx-signal";
inline constexpr std::string_view EncoderId="tzx-signal";
inline constexpr std::string_view FormatId="tape.tzx",WriterId="tape-tzx",TzxExtension=".tzx",CdtExtension=".cdt",TsxExtension=".tsx",PayloadKey="payloadLength",KnownKey="known",MajorKey="majorVersion",MinorKey="minorVersion",ContextKey="contextExtension",TrueValue="True",FalseValue="False";
inline constexpr int SignatureSize=8,MarkerOffset=7,MajorOffset=8,MinorOffset=9,ByteWidth=1,WordWidth=2,TripleWidth=3,DwordWidth=4,TurboHeaderSize=18,TurboLengthOffset=15,PureHeaderSize=10,PureLengthOffset=7,DirectHeaderSize=8,DirectLengthOffset=5,CustomHeaderSize=20,CustomLengthOffset=16,GlueLength=9,DecimalRadix=10,HexRadix=16,ByteBits=8,HexWidth=2,MaximumByte=255;
inline constexpr char DigitZero='0';
inline constexpr std::array<std::uint8_t,25> SupportedBlockIds{StandardSpeedData,TurboSpeedData,PureTone,PulseSequence,PureData,DirectRecording,CswRecording,GeneralizedData,Pause,GroupStart,GroupEnd,Jump,LoopStart,LoopEnd,CallSequence,Return,Select,StopIf48K,SetSignalLevel,TextDescription,Message,ArchiveInfo,HardwareType,CustomInfo,Glue};
inline constexpr std::string_view SourcePositionKey="sourceSegmentPosition",RoleKey="role",PulseStatesKey="pulseTStates",PulseCountKey="pulseCount";
inline constexpr std::string_view PilotRole="pilot",SyncRole="sync",ToneRole="tone",SequenceRole="sequence",DataRole="encoded-bits",DirectRole="direct-recording",StopRole="stop",PauseRole="pause",SpectrumMachine="spectrum",MsxMachine="msx";
inline constexpr int StandardHeaderSize=4,StandardLengthOffset=2,HeaderFlagBoundary=128,BitMask=7,PulsePair=2,MillisecondsPerSecond=1000,MaximumWord=65535;
inline constexpr int TurboSyncFirstOffset=2,TurboSyncSecondOffset=4,TurboZeroOffset=6,TurboOneOffset=8,TurboPilotCountOffset=10,TurboUsedBitsOffset=12,TurboPauseOffset=13;
inline constexpr int PureOneOffset=2,PureUsedBitsOffset=4,PurePauseOffset=5,DirectPauseOffset=2,DirectUsedBitsOffset=4;
}
