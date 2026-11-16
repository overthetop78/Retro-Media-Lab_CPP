#pragma once
#include <RetroMediaLab/MediaFileSystems/FamicomDiskMetadataConstants.h>
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Cartridge::Constants {
inline constexpr std::int32_t CprHeaderLength=12,ChunkHeaderLength=8,BankLength=16384,MaximumBankCount=32,SignatureLength=4,SizeOffset=4,FormOffset=8,BankNumberOffset=2,NameDigits=2,OptionalRomHeaderLength=128;
inline constexpr std::int32_t RomChecksumLast=66,RomChecksumLow=67,RomChecksumHigh=68,CommandLength=4,CommandRepeat=3;
inline constexpr std::uint8_t CommandByte=0xfd,CommandFirst=0x70,CommandSecond=0x71,CommandThird=0x77,AsciiMaximum=127;
inline constexpr std::uint32_t RomChecksumMask=65535;
inline constexpr std::array<std::int64_t,8> RomCorpusLengths={8192,12101,16383,16384,16512,32768,131072,524288};
inline constexpr std::int32_t FdsHeaderLength=16,FdsSideLength=65500,FdsSideCountOffset=4;
inline constexpr std::array<std::uint8_t,4> FdsSignature={'F','D','S',0x1a};
inline constexpr char Zero='0',AsciiReplacement='?';
inline constexpr std::string_view Riff="RIFF",Form="AMS!",CprBankPrefix="cb",CprExtension=".cpr",RomExtension=".rom",BinExtension=".bin",FdsExtension=".fds";
inline constexpr std::string_view SystemIdKey="systemId",RoleKey="mediaRole",BankSizeKey="bankSize",BankCountKey="bankCount",SourceHeaderLengthKey="sourceHeaderLength",BankPrefix="bank.",BankLengthSuffix=".length",ChunkIdSuffix=".chunkId";
inline constexpr std::string_view HeaderedKey=MediaFileSystems::FamicomDiskMetadataConstants::Headered,HeaderLengthKey=MediaFileSystems::FamicomDiskMetadataConstants::HeaderLength,SideCountKey=MediaFileSystems::FamicomDiskMetadataConstants::SideCount,DeclaredSideCountKey=MediaFileSystems::FamicomDiskMetadataConstants::DeclaredSideCount,SideLengthKey=MediaFileSystems::FamicomDiskMetadataConstants::SideLength,SidePrefix=MediaFileSystems::FamicomDiskMetadataConstants::SideNamePrefix,SideNameSuffix=MediaFileSystems::FamicomDiskMetadataConstants::SideNameSuffix,SideNamePrefix="side",True="True",False="False";
inline constexpr std::string_view AmstradSystem="amstrad",NintendoSystem="nintendo",CartridgeRole="cartridge",RomRole="rom",FloppyRole="floppy",Domain="MediaEngine";
inline constexpr std::string_view CprVolumeKey="MediaEngine.AmstradCartridgeVolume",RomVolumeKey="MediaEngine.AmstradRomVolume",FdsVolumeKey="MediaEngine.FamicomVolume";
inline constexpr std::string_view CprWriterId="amstrad-cpr",RomWriterId="amstrad-rom",FdsWriterId="nintendo-famicom-disk";
}
