#pragma once
#include <cstdint>
#include <array>
#include <string_view>
namespace RetroMediaLab::MediaEngine::AppleRwts18::Constants {
inline constexpr std::int32_t PageByteCount=256,PageCount=3,SectorByteCount=768,PayloadSymbolCount=1024,PayloadWithChecksumSymbolCount=1025,SymbolsPerPageGroup=4,SecondPageIndex=1,ThirdPageIndex=2,FirstPageHighBitShift=2,SecondPageHighBitShift=4,ThirdPageHighBitShift=6,SourceHighBitShift=6,FirstPagePackedShift=4,SecondPagePackedShift=2,HighSymbolOffset=0,FirstPageSymbolOffset=1,SecondPageSymbolOffset=2,ThirdPageSymbolOffset=3,PayloadSectorNumber=-1;
inline constexpr std::uint8_t SixBitMask=0x3f,HighBitMask=0xc0;
inline constexpr std::string_view Id="apple2.rwts18",DisplayName="Apple II Brøderbund RWTS18",IdentifierAttribute="id",AddressKey="MediaEngine.Rwts18Address",DataKey="MediaEngine.Rwts18Data",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid";
inline constexpr std::uint16_t EncodedAddressMark=0xd59d;
inline constexpr std::int32_t AddressMarkBitCount=16,AddressMarkAdvanceBitCount=15,AddressByteCount=4,SeparatorByteCount=3,SectorCount=6,LastSectorNumber=5,PayloadOffset=1,DataRecordByteCount=1027,DataEpilogueOffset=1026,DataReadWindowByteCount=1100,CircularTailBitCount=16384,FirstSectorGapBitCount=200,OtherSectorGapBitCount=4,MaximumCylinder=63,MaximumIdentifier=255,AddressTrackOffset=0,AddressSectorOffset=1,AddressChecksumOffset=2,AddressTrailerOffset=3,BitsPerByte=8;
inline constexpr std::uint8_t AddressTrailer=0xaa,DataEpilogue=0xd4,SyncByte=0xff,DefaultIdentifier=0xa4,LogicalHead=0,SectorSizeCode=3;
inline constexpr double ConfidenceCompleteSectorDivisor=6,ConfidenceDetectedSectorDivisor=24,MaximumConfidence=1;
inline constexpr std::array<std::uint8_t,2> AddressMarkBytes{0xd5,0x9d},DataTrailerBytes{DataEpilogue,SyncByte};
}
