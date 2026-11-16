#pragma once
#include <RetroMediaLab/MediaEngine/TrackEncodingAttributeKeys.h>
#include <cstdint>
#include <array>
#include <string_view>
namespace RetroMediaLab::MediaEngine::AppleIwm::Constants {
inline constexpr std::int32_t TagByteCount=12,SectorByteCount=512,TaggedSectorByteCount=524,EncodedPayloadSymbolCount=699,ChecksumSymbolCount=4,GroupByteCount=175,LastGroupIndex=174,SymbolsPerGroup=4,FirstChecksumShift=6,SecondChecksumShift=4,ThirdChecksumShift=2,FirstPackedChecksumMask=0x30,SecondPackedChecksumMask=0x0c,ThirdPackedChecksumMask=0x03;
inline constexpr std::uint8_t SixBitMask=0x3f,EncodedHighBitsMask=0xc0;
inline constexpr std::uint32_t ChecksumByteMask=0xff,ChecksumCarryBit=0x100,ChecksumHighBitsMask=0xc0;
inline constexpr std::string_view MacId="applemac.gcr",MacName="Apple Macintosh GCR",LisaId="applelisa.fileware.gcr",LisaName="Apple Lisa FileWare GCR",FormatAttribute=TrackEncodingAttributeKeys::Format,TagPrefix=TrackEncodingAttributeKeys::TagPrefix,FormatField="formatValue",AddressKey="MediaEngine.IwmAddress",DataKey="MediaEngine.IwmData",TruncatedKey="MediaEngine.IwmTruncated",UnpairedKey="MediaEngine.IwmUnpaired",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid",UnavailableKey="MediaEngine.IntegrityUnavailable";
inline constexpr std::int32_t MarkBitCount=24,MarkAdvanceBitCount=23,HeaderSymbolCount=5,HeaderValueCount=4,DataSymbolCount=704,DataSectorSymbolOffset=0,DataPayloadOffset=1,PackedChecksumSymbolOffset=700,ThirdChecksumSymbolOffset=701,SecondChecksumSymbolOffset=702,FirstChecksumSymbolOffset=703,CylinderHighBitMask=3,CylinderHighBitShift=6,HeadBitShift=5,HeadBitMask=1,DataSearchBitCount=512,CircularTailBitCount=8192,AddressLeadingGapBitCount=100,AddressTrailingGapBitCount=32,DataTrailingGapBitCount=64,MaximumCylinder=255,MaximumHead=1,MaximumTagValue=255,HeaderCylinderOffset=0,HeaderSectorOffset=1,HeaderHeadOffset=2,HeaderFormatOffset=3,HeaderChecksumOffset=4,BitsPerByte=8,ConfidenceSectorWeight=2,ChecksumFirstOffset=0,ChecksumSecondOffset=1,ChecksumThirdOffset=2,ChecksumPackedOffset=3;
inline constexpr std::uint8_t SectorSizeCode=2,DefaultFormat=0x12;
inline constexpr double ConfidenceDivisor=24;
inline constexpr std::array<std::uint8_t,3> AddressMark{0xd5,0xaa,0x96},DataMark{0xd5,0xaa,0xad},DataEpilogue{0xde,0xaa,0xff};
inline constexpr std::array<std::uint8_t,4> AddressEpilogue{0xde,0xaa,0xff,0xff};
}
