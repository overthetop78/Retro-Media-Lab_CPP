#pragma once
#include "SectorImage.h"
#include "FileCoreLayout.h"
namespace RetroMediaLab::MediaFileSystems::FileCore {
struct DiscRecord {
    std::int32_t log2SectorSize, idLength, log2BytesPerMapBit, zoneCount, zoneSpareBits, rootAddress;
    std::uint64_t discSize;
    std::string discName;
    std::int32_t log2ShareSize;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::int32_t zoneSizeBits() const;
};
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool validChecksum(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<std::uint64_t> oldMapDeclaredCapacity(Bytes);
struct Address { std::uint32_t fragmentId, shareOffset; };
RETROMEDIALAB_MEDIAFILESYSTEMS_API Address decodeAddress(std::int32_t) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<DiscRecord> parseDiscRecord(Bytes, std::uint64_t imageCapacity);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::uint32_t getBits(Bytes, std::int64_t bitOffset, std::uint32_t mask);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::int64_t findNextSetBit(Bytes, std::int64_t start, std::int64_t end);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::int32_t applyShift(std::int32_t value, std::int32_t shift);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string decodeDiscName(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string decodeAdfsName(Bytes);
class AddressResolver {
public:
    virtual ~AddressResolver() = default;
    virtual std::int32_t rootAddress() const noexcept = 0;
    virtual const std::string& volumeName() const noexcept = 0;
    virtual std::uint64_t freeBytes() const noexcept = 0;
    virtual std::optional<std::uint64_t> resolve(std::int32_t indirectAddress, std::int64_t objectByteOffset, std::stop_token = {}) const = 0;
};
struct OldMapOptions { std::int32_t rootAddress, cylinders = 0, heads = 1, sectorsPerTrack = 0; };
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::unique_ptr<AddressResolver> oldMap(Bytes, std::uint64_t capacity);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::unique_ptr<AddressResolver> oldMap(Bytes, std::uint64_t capacity, OldMapOptions);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::unique_ptr<AddressResolver> newMap(const SectorImage&, std::stop_token = {});
}
