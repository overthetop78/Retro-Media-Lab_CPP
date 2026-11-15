#pragma once
#include <cstdint>
#include <RetroMediaLab/MediaFileSystems/FileCoreLayout.h>
namespace RetroMediaLab::MediaFileSystems::FileCore::Constants {
inline constexpr int BitsPerByte = 8;
inline constexpr int ByteMask = 255;
inline constexpr int SevenBitMask = 127;
inline constexpr char AsciiReplacement = '?';
inline constexpr char Space = ' ';
inline constexpr char CarriageReturn = '\r';
inline constexpr int UInt24Size = 3;
inline constexpr int WordBits = 32;
inline constexpr int WordBytes = 4;
inline constexpr int PairCount = 2;
inline constexpr int RecordLength = Layout::DiscRecordLength;
inline constexpr int RecordOffset = Layout::DiscRecordOffset;
inline constexpr int IdLengthOffset = 4;
inline constexpr int MapBitOffset = 5;
inline constexpr int ZonesLowOffset = 9;
inline constexpr int ZonesHighOffset = 42;
inline constexpr int ZoneSpareOffset = 10;
inline constexpr int RootOffset = 12;
inline constexpr int SizeLowOffset = 16;
inline constexpr int SizeHighOffset = 36;
inline constexpr int NameOffset = 22;
inline constexpr int NameLength = 10;
inline constexpr int ShareOffset = 40;
inline constexpr int ShareMask = 15;
inline constexpr int MinLogSector = 8;
inline constexpr int MaxLogSector = 10;
inline constexpr int MinExtraIdBits = 3;
inline constexpr int MaxIdBits = 19;
inline constexpr int BootAddress = Layout::BootBlockAddress;
inline constexpr int BootLength = 512;
inline constexpr int BootRecordOffset = Layout::BootDiscRecordOffset;
inline constexpr int RootFragment = 2;
inline constexpr int ZoneHeaderBits = 32;
inline constexpr int FreeLinkBit = 8;
inline constexpr int FreeMask = 0x7fff;
inline constexpr int MaxFreeIdBits = 15;
inline constexpr int ZoneTerminatorOffset = 2;
inline constexpr int ZoneTerminatorMask = 0x80;
inline constexpr int ZoneCrossCheckOffset = 3;
inline constexpr int CrossCheck = 0xff;
inline constexpr int FileCoreUnitSize = 256;
inline constexpr int OldRootAddress = 4;
inline constexpr int FirstNameOffset = 247;
inline constexpr int SecondNameOffset = 502;
inline constexpr int HalfNameLength = 5;
inline constexpr int TotalUnitsOffset = 0xfc;
inline constexpr int FreeEntriesOffset = 256;
inline constexpr int FreeEntryCount = 82;
}
