#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::Apple::RecognitionConstants {
inline constexpr std::size_t BlockSize=512,MacSignatureOffset=1024,SignatureBytes=2;
inline constexpr std::uint16_t MfsSignature=0xD2D7,HfsSignature=0x4244;
inline constexpr std::size_t DosSectorSize=256,DosVtocTrack=17,DosCatalogTrack=1,DosCatalogSector=2,DosTrackSectors=0x35,DosSectorBytes=0x36;
inline constexpr std::size_t ProDosRootBlock=2,ProDosHeader=4,ProDosEntryLength=0x23,ProDosEntrySize=0x27;
inline constexpr std::uint8_t ProDosStorageShift=4,ProDosNameMask=0x0f,ProDosVolumeHeader=0x0f;
inline constexpr std::size_t LisaCapacity=800*BlockSize,LisaMinimumLength=64,LisaVersionOffset=0,LisaNameLength=12,LisaNameOffset=13,LisaMaximumName=31;
inline constexpr std::array<std::uint16_t,3> LisaVersions{0x000e,0x000f,0x0011};
inline constexpr std::uint8_t PrintableFirst=0x20,PrintableLast=0x7e;
inline constexpr char Space=' ',Null='\0';
inline constexpr std::size_t SosSearchLength=128,SosMarkerOffset=8;
inline constexpr std::string_view SosMarker="SOS";
inline constexpr std::uint8_t LowerFirst='a',LowerLast='z',CaseDelta='a'-'A';
}
