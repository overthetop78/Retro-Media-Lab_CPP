#pragma once
#include "LocalizedText.h"
#include <cstdint>
#include <optional>
namespace RetroMediaLab::MediaEngine::Decoding {
enum class StructureKind {Sync,IdAddressMark,DataAddressMark,DeletedDataAddressMark,AmigaSync,AppleAddress,AppleData,CommodoreSync,CommodoreHeader,FormatHeader,FormatData,TimingAnomaly};
enum class IntegrityKind {Crc,Checksum};
struct Structure {StructureKind kind;std::uint64_t bitOffset,bitLength;LocalizedText description;};
struct Sector {
 std::uint8_t cylinder=0,head=0,sizeCode=0;std::int32_t number=0;std::uint32_t sizeBytes=0;std::optional<bool> integrityValid;std::uint64_t bitOffset=0;IntegrityKind integrityKind=IntegrityKind::Crc;
 std::optional<std::vector<std::uint8_t>> data,tag;std::optional<std::uint8_t> formatCode;
 std::vector<std::uint8_t> recordedHeaderErrorCheckCode,recordedErrorCheckCode;
 std::optional<bool> headerIntegrityValid,dataIntegrityValid,deletedDataMark;
};
struct Result {std::string decoderId,displayName;double confidence=0,estimatedCellTicks=0;std::vector<Structure> structures;std::vector<std::uint8_t> decodedBytes;std::vector<Sector> sectors;};
}
