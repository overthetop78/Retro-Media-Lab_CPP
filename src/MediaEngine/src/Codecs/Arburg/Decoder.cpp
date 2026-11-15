#include "Blocks.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Primitives/Bits.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>

namespace RetroMediaLab::MediaEngine::Arburg {
namespace {
Decoding::LocalizedText state(std::optional<bool> valid) {
    return Decoding::message(valid ? (*valid ? Constants::ValidKey : Constants::InvalidKey)
        : Constants::UnavailableKey);
}

void append(Decoding::Result& result, bool system, std::int64_t offset,
    std::int64_t length, std::optional<std::vector<std::uint8_t>> block, std::stop_token token) {
    const auto total = system ? Constants::SystemSize : Constants::DataSize;
    const auto useful = system ? Constants::SystemUsefulSize : Constants::DataUsefulSize;
    std::optional<bool> valid;
    std::optional<std::vector<std::uint8_t>> payload;
    if (block) {
        valid = checksumValid(*block, useful, token);
        payload = std::vector<std::uint8_t>(block->begin(), block->begin() + useful);
        result.decodedBytes.insert(result.decodedBytes.end(), payload->begin(), payload->end());
    }
    Decoding::Sector sector;
    sector.cylinder = Constants::LogicalCylinder;
    sector.head = Constants::LogicalHead;
    sector.number = Constants::LogicalSector;
    sector.sizeCode = Constants::SizeCode;
    sector.sizeBytes = total;
    sector.integrityValid = valid;
    sector.bitOffset = offset;
    sector.integrityKind = Decoding::IntegrityKind::Checksum;
    sector.data = std::move(payload);
    result.sectors.push_back(std::move(sector));
    result.structures.push_back({
        system ? Decoding::StructureKind::FormatHeader : Decoding::StructureKind::FormatData,
        std::uint64_t(offset), std::uint64_t(length),
        Decoding::message(system ? Constants::SystemKey : Constants::DataKey, {state(valid)})});
}
}

Decoding::Result decode(const Flux::Revolution& revolution, std::stop_token token) {
    checkCancellation(token);
    const auto stream = Flux::decodeAdaptiveMfm(revolution.intervals, token);
    const auto bits = stream.bits();
    Decoding::Result result;
    result.decoderId = Constants::Id;
    result.displayName = Constants::DisplayName;
    result.estimatedCellTicks = stream.cellTicks();

    for (std::int64_t offset = 0; offset + Constants::MarkBits <= std::int64_t(bits.size()); ++offset) {
        checkCancellation(token);
        if (!Flux::matchBytes(bits, offset, Constants::DataMark)) continue;
        const auto end = offset + Constants::MarkBits
            + std::int64_t(Constants::DataSize) * Constants::FmByteBits;
        std::optional<std::vector<std::uint8_t>> block;
        if (end <= std::int64_t(bits.size())) {
            block.emplace();
            block->reserve(Constants::DataSize);
            for (int index = 0; index < Constants::DataSize; ++index) {
                checkCancellation(token);
                const auto value = Flux::fmByte32(bits, offset + Constants::MarkBits
                    + std::int64_t(index) * Constants::FmByteBits);
                if (!value) { block.reset(); break; }
                block->push_back(Primitives::reverseBits(*value));
            }
            if (!block) continue;
        }
        append(result, false, offset, block ? end - offset : Constants::MarkBits,
            std::move(block), token);
        offset += Constants::MarkBits - 1;
    }
    for (std::int64_t offset = 0; offset + Constants::MarkBits <= std::int64_t(bits.size()); ++offset) {
        checkCancellation(token);
        if (!Flux::matchBytes(bits, offset, Constants::SystemMark)) continue;
        auto decoded = decodeSystem(bits, offset + Constants::MarkBits, Constants::SystemSize, token);
        const auto length = decoded ? decoded->endOffset - offset : Constants::MarkBits;
        std::optional<std::vector<std::uint8_t>> block;
        if (decoded) block = std::move(decoded->bytes);
        append(result, true, offset, length, std::move(block), token);
        offset += Constants::MarkBits - 1;
    }
    std::stable_sort(result.structures.begin(), result.structures.end(),
        [](const auto& left, const auto& right) { return left.bitOffset < right.bitOffset; });
    result.confidence = Decoding::confidence(std::int32_t(result.sectors.size()),
        std::int32_t(result.structures.size()), Constants::ConfidenceWeight, Constants::ConfidenceDivisor);
    return result;
}
}

