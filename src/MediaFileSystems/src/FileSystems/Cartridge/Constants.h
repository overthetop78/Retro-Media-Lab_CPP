#pragma once
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <array>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::CartridgeConstants {
inline constexpr int FirstBank=0, MaximumBankCount=4096, AmstradBankCount=32, AmstradBankLength=16384, NameDigits=2;
inline constexpr char Null='\0', Zero='0', LastDigit='9';
inline constexpr std::string_view BankCount="bankCount", BankSize="bankSize", BankPrefix="bank.", LengthSuffix=".length", NameSuffix=".name", ChunkIdSuffix=".chunkId", DefaultNamePrefix="bank";
inline constexpr std::string_view BankEntryType="cartridge-bank", BankAttribute="bank", BankNumberMetadata="bankNumber", AddressMetadata="address", BanksAttribute="cartridge-banks";
inline constexpr std::string_view AmstradDisplayKey="MediaFileSystems.AmstradCartridgeBanks", TranslationDomain="MediaFileSystems";
inline constexpr std::array ConsoleFormats={Formats::NintendoNes,Formats::NintendoSnes,Formats::NintendoN64,Formats::NintendoGameBoy,Formats::NintendoGameBoyColor,Formats::NintendoGameBoyAdvance,Formats::NintendoNds,Formats::NintendoGameWatch,Formats::Nintendo3Ds,Formats::NintendoVirtualBoy,Formats::SegaSg1000,Formats::SegaMasterSystem,Formats::SegaMegaDrive,Formats::SegaGameGear,Formats::SegaThirtyTwoX,Formats::NecPcEngine,Formats::NecSuperGrafx,Formats::Atari2600,Formats::Atari5200,Formats::Atari7800,Formats::BandaiWonderSwan};
inline constexpr std::array AmstradFormats={Formats::AmstradCpr,Formats::AmstradRom};
}
