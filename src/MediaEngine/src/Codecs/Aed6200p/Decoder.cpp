#include "Mfm.h"
#include "../Crc16.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Models/Sectors/SizeCodes.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <unordered_set>

namespace RetroMediaLab::MediaEngine::Aed6200p {
namespace {
using namespace Constants;

Decoding::LocalizedText state(std::optional<bool> valid) {
    return Decoding::message(valid ? (*valid ? ValidKey : InvalidKey) : UnavailableKey);
}

bool isDataMark(Flux::BitView bits, std::int64_t offset) {
    return std::any_of(DataMarks.begin(), DataMarks.end(),
        [&](const auto& mark) { return Flux::matchBytes(bits, offset, mark.pattern); });
}

std::int64_t findDataMark(Flux::BitView bits, std::int64_t start, std::int64_t end,
    std::stop_token token) {
    for (auto offset = std::max(std::int64_t(0), start); offset + MarkBitCount <= end; ++offset) {
        checkCancellation(token);
        if (isDataMark(bits, offset)) return offset;
    }
    return -1;
}

std::string hexByte(std::uint8_t value) {
    return {HexDigits[value >> NibbleBits], HexDigits[value & NibbleMask]};
}

struct DataResult {
    bool headerCanBeAdded;
    std::optional<bool> valid;
    std::int64_t structureEnd;
    std::optional<std::vector<std::uint8_t>> payload;
};

DataResult readData(Flux::BitView bits, std::int64_t headerOffset, std::int32_t size,
    std::unordered_set<std::int64_t>& paired, Decoding::Result& result, std::stop_token token) {
    const auto dataOffset = findDataMark(bits, headerOffset + 1,
        std::min(std::int64_t(bits.size()), headerOffset + SearchWindowBitCount), token);
    const auto headerEnd = headerOffset + HeaderBitCount;
    if (dataOffset < 0) return {true, {}, headerEnd, {}};

    paired.insert(dataOffset);
    const auto byteCount = DataMarkByteCount + size + CrcByteCount;
    const auto dataEnd = dataOffset + std::int64_t(byteCount) * EncodedByteBitCount;
    if (size <= 0 || dataEnd > std::int64_t(bits.size())) {
        result.structures.push_back({Decoding::StructureKind::FormatData,
            std::uint64_t(dataOffset), EncodedByteBitCount, Decoding::message(TruncatedDataKey)});
        return {true, {}, headerEnd, {}};
    }
    const auto data = Flux::mfmBytes(bits, dataOffset, byteCount, token);
    if (!data) return {false, {}, headerEnd, {}};
    const auto mark = (*data)[HeaderMarkOffset];
    const auto valid = mark >= FirstDataMark && mark <= LastDataMark
        && Crc16::compute(*data, Crc16::Constants::Ccitt,
            Crc16::Constants::AllBitsSet, token) == 0;
    std::vector<std::uint8_t> payload(data->begin() + DataMarkByteCount,
        data->begin() + DataMarkByteCount + size);
    result.decodedBytes.insert(result.decodedBytes.end(), payload.begin(), payload.end());
    result.structures.push_back({Decoding::StructureKind::FormatData,
        std::uint64_t(dataOffset), std::uint64_t(dataEnd - dataOffset),
        Decoding::message(DataKey, {Decoding::literal(std::to_string(size)),
            Decoding::literal(hexByte(mark)), state(valid)})});
    return {true, valid, dataEnd, std::move(payload)};
}
}

Decoding::Result decode(const Flux::Revolution& revolution, std::stop_token token) {
    using namespace Constants;
    checkCancellation(token);
    const auto stream = Flux::decodeAdaptiveMfm(revolution.intervals, token);
    const auto bits = stream.bits();
    Decoding::Result result;
    result.decoderId = Id;
    result.displayName = DisplayName;
    result.estimatedCellTicks = stream.cellTicks();
    std::unordered_set<std::int64_t> paired;

    for (std::int64_t offset = 0; offset + MarkBitCount <= std::int64_t(bits.size()); ++offset) {
        checkCancellation(token);
        if (!Flux::matchBytes(bits, offset, HeaderPattern)) continue;
        if (offset + HeaderBitCount > std::int64_t(bits.size())) {
            result.structures.push_back({Decoding::StructureKind::FormatHeader,
                std::uint64_t(offset), MarkBitCount, Decoding::message(TruncatedHeaderKey)});
            offset += MarkBitCount - 1;
            continue;
        }
        const auto header = Flux::mfmBytes(bits, offset, HeaderByteCount, token);
        if (!header) continue;
        const auto size = std::int32_t(((*header)[SizeHighOffset] << BitsPerByte)
            | (*header)[SizeLowOffset]);
        const auto headerValid = (*header)[HeaderMarkOffset] == HeaderMark
            && Crc16::compute(*header, Crc16::Constants::Ccitt,
                Crc16::Constants::AllBitsSet, token) == 0;
        result.decodedBytes.insert(result.decodedBytes.end(), header->begin(), header->end());
        auto data = readData(bits, offset, size, paired, result, token);
        if (!data.headerCanBeAdded) continue;

        Decoding::Sector sector;
        sector.cylinder = (*header)[CylinderOffset];
        sector.head = LogicalHead;
        sector.number = (*header)[SectorOffset];
        sector.sizeCode = trySectorSizeCode(size).value_or(0);
        sector.sizeBytes = size;
        sector.bitOffset = offset;
        if (!headerValid || (data.valid && !*data.valid)) sector.integrityValid = false;
        else if (data.valid) sector.integrityValid = true;
        sector.data = std::move(data.payload);
        result.sectors.push_back(std::move(sector));
        result.structures.push_back({Decoding::StructureKind::FormatHeader,
            std::uint64_t(offset), HeaderBitCount,
            Decoding::message(HeaderKey, {
                Decoding::literal(std::to_string((*header)[CylinderOffset])),
                Decoding::literal(std::to_string(LogicalHead)),
                Decoding::literal(std::to_string((*header)[SectorOffset])),
                Decoding::literal(std::to_string(size)), state(headerValid), state(data.valid)})});
        offset = std::max(offset + MarkBitCount - 1, data.structureEnd - 1);
    }
    for (std::int64_t offset = 0; offset + MarkBitCount <= std::int64_t(bits.size()); ++offset) {
        checkCancellation(token);
        if (!isDataMark(bits, offset) || paired.contains(offset)) continue;
        result.structures.push_back({Decoding::StructureKind::FormatData,
            std::uint64_t(offset), EncodedByteBitCount, Decoding::message(UnpairedKey)});
        offset += EncodedByteBitCount - 1;
    }
    result.confidence = Decoding::standardConfidence(std::int32_t(result.sectors.size()),
        std::int32_t(result.structures.size()));
    return result;
}
}

