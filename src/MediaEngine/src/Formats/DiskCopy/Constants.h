#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
namespace RetroMediaLab::MediaEngine::DiskCopy::Constants {
inline constexpr std::size_t NameLengthOffset=0,NameOffset=1,MaximumNameLength=63,MinimumDataLength=1,HeaderSize=84;
inline constexpr std::size_t DataLengthOffset=64,TagLengthOffset=68,DataChecksumOffset=72,TagChecksumOffset=76,DiskFormatOffset=80,FormatByteOffset=81,PrivateWordOffset=82;
inline constexpr std::size_t DataBlockSize=512,TagSize=12,TagChecksumExcludedPrefixSize=TagSize,PrebootSearchBlockIndex=2,PrebootSearchLength=16;
inline constexpr std::uint16_t PrivateWord=0x0100;
inline constexpr std::uint32_t MissingChecksum=0,InitialChecksum=0;
inline constexpr std::size_t ChecksumWordSize=sizeof(std::uint16_t);
inline constexpr int ChecksumRotation=1,BitsPerByte=8;
inline constexpr std::uint8_t DiskFormat400K=0,DiskFormat800K=1,DiskFormat720K=2,DiskFormat1440K=3,FormatMfs=0x22,FormatHfs=0x24,FormatLisa=0x02;
inline constexpr std::array<std::uint8_t,7> PrebootMarker{0x50,0x52,0x45,0x42,0x4f,0x4f,0x54};
inline constexpr auto MaximumLength=std::numeric_limits<std::int32_t>::max();
inline constexpr std::string_view AppleLisaOffice="applelisa.office",AppleLisaMacWorks="applelisa.macworks",LisaPrefix="applelisa.",MfsMarker="mfs",ImageExtension=".image";
inline constexpr std::string_view NameKey="diskcopy.nameBytes",DiskFormatKey="diskcopy.diskFormat",FormatByteKey="diskcopy.formatByte",DataChecksumKey="diskcopy.dataChecksumStatus",TagChecksumKey="diskcopy.tagChecksumStatus";
inline constexpr std::string_view Valid="valid",Invalid="invalid",Missing="missing",Base64Whitespace=" \t\r\n";
inline constexpr char FirstDigit='0',LastDigit='9',AsciiReplacement='?';
inline constexpr int DecimalBase=10,MaximumByte=255,AsciiMaximum=127,Base64QuantumSize=4;
}
