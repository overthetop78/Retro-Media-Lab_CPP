#pragma once
#include <cstdint>
#include <cstddef>
#include <string_view>
namespace RetroMediaLab::MediaEngine::AtomTap::Constants {
inline constexpr int FileHeaderBytes=22;
inline constexpr int FileNameBytes=16;
inline constexpr int LoadAddressOffset=16;
inline constexpr int ExecutionAddressOffset=18;
inline constexpr int LengthOffset=20;
inline constexpr int MaximumBlockBytes=256;
inline constexpr int SyncBytes=4;
inline constexpr std::uint8_t Sync=0x2a;
inline constexpr std::uint8_t NameTerminator=0x0d;
inline constexpr std::uint8_t MoreBlocks=0x80;
inline constexpr std::uint8_t ContainsData=0x40;
inline constexpr std::uint8_t Continuation=0x20;
inline constexpr int EndAddressShift=11;
inline constexpr int FlagHistoryShift=2;
inline constexpr int FlagHistoryMask=0x1f;
inline constexpr int Baud=300;
inline constexpr int StopBits=1;
inline constexpr int DataBits=8;
inline constexpr std::uint8_t NoParity='N';
inline constexpr int CarrierCycles=9600;
inline constexpr int HeaderDataCarrierCycles=1200;
inline constexpr int GapSeconds=2;
inline constexpr int InitialPhase=180;
inline constexpr std::string_view FormatId="tape.atom-tap",Extension=".tap",WriterId="tape-atom-tap",DecoderId="acorn-atom-tape",DefinedChunk="0104",BaudChunk="0117",FileNameKey="fileName",BlockNumberKey="blockNumber",LoadKey="loadAddress",ExecuteKey="executionAddress",FirstBlock="0";
inline constexpr std::size_t SerialPrefix=3,StopOffset=2,HeaderTail=9,FlagsAfterName=1,NumberAfterName=2,CountAfterName=4,ExecuteAfterName=5,LoadAfterName=7,CountBias=1,WordBytes=2,FloatBytes=4,MaximumFileLength=65535,MaximumBuffer=0x7fffffff;
inline constexpr int AlternateStopBits=2,HexWidth=4,HexRadix=16;
inline constexpr std::uint8_t MaximumAscii=127;
inline constexpr char AsciiReplacement='?';
inline constexpr char FirstPrintable=' ',LastPrintable='~',DigitZero='0';
}
