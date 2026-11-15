#include "Mfm.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::QdMo5 {
namespace {
Decoding::LocalizedText state(std::optional<bool> valid) {
    return Decoding::message(valid ? (*valid ? Constants::ValidKey : Constants::InvalidKey) : Constants::UnavailableKey);
}
std::string hexByte(std::uint8_t value) {
    return {Constants::HexDigits[value >> Constants::NibbleBits], Constants::HexDigits[value & Constants::NibbleMask]};
}
}
Decoding::Result decode(const Flux::Revolution& revolution, std::stop_token token) {
    using namespace Constants;
    checkCancellation(token);
    const auto stream = Flux::decodeAdaptiveMfm(revolution.intervals, token);
    const auto bits = stream.bits();
    Decoding::Result result;
    result.decoderId = Id; result.displayName = DisplayName; result.estimatedCellTicks = stream.cellTicks();
    std::unordered_set<std::int64_t> paired;
    for (std::int64_t offset = 0; offset + MarkBits <= std::int64_t(bits.size()); ++offset) {
        checkCancellation(token);
        if (!Flux::matchBytes(bits, offset, HeaderMark)) continue;
        const auto header = decodeHeader(bits, offset, token);
        if (!header) {
            result.structures.push_back({Decoding::StructureKind::FormatHeader,
                std::uint64_t(offset), MarkBits, Decoding::message(TruncatedHeaderKey)});
            offset += MarkBits - 1; continue;
        }
        result.decodedBytes.push_back(std::uint8_t(header->sector >> BitsPerByte));
        result.decodedBytes.push_back(std::uint8_t(header->sector));
        const auto dataOffset = findNextData(bits, offset + HeaderBits, SearchBits, token);
        const auto data = dataOffset < 0 ? std::optional<Data>{} : decodeData(bits, dataOffset, token);
        if (dataOffset >= 0) {
            paired.insert(dataOffset);
            if (!data)
                result.structures.push_back({Decoding::StructureKind::FormatData,
                    std::uint64_t(dataOffset), MarkBits, Decoding::message(TruncatedDataKey)});
            else {
                result.decodedBytes.insert(result.decodedBytes.end(), data->payload.begin(), data->payload.end());
                result.structures.push_back({Decoding::StructureKind::FormatData,
                    std::uint64_t(dataOffset), DataBits, Decoding::message(DataKey, {
                        Decoding::literal(std::to_string(header->sector)), Decoding::literal(hexByte(data->prefix)), state(data->checksumValid)})});
            }
        }
        Decoding::Sector sector;
        sector.cylinder = LogicalCylinder; sector.head = LogicalHead; sector.number = header->sector;
        sector.sizeCode = SectorSizeCode; sector.sizeBytes = SectorSize; sector.bitOffset = offset;
        sector.integrityKind = Decoding::IntegrityKind::Checksum;
        if (data) { sector.integrityValid = data->checksumValid; sector.data = data->payload; }
        result.sectors.push_back(std::move(sector));
        result.structures.push_back({Decoding::StructureKind::FormatHeader,
            std::uint64_t(offset), HeaderBits, Decoding::message(HeaderKey, {
                Decoding::literal(std::to_string(header->sector)), state(true),
                state(data ? std::optional<bool>(data->checksumValid) : std::nullopt)})});
        offset += MarkBits - 1;
    }
    for (std::int64_t offset = 0; offset + MarkBits <= std::int64_t(bits.size()); ++offset) {
        checkCancellation(token);
        if (!Flux::matchBytes(bits, offset, Preamble)) continue;
        const auto mark = Flux::mfmByte(bits, offset + PreambleBits);
        if (!mark || *mark == HeaderAddressMark || paired.contains(offset)) continue;
        result.structures.push_back({Decoding::StructureKind::FormatData,
            std::uint64_t(offset), MarkBits, Decoding::message(UnpairedKey, {Decoding::literal(hexByte(*mark))})});
        offset += MarkBits - 1;
    }
    std::stable_sort(result.structures.begin(), result.structures.end(),
        [](const auto& left, const auto& right) { return left.bitOffset < right.bitOffset; });
    result.confidence = Decoding::confidence(std::int32_t(result.sectors.size()),
        std::int32_t(result.structures.size()), ConfidenceWeight, ConfidenceDivisor);
    return result;
}
}

