#include "Fm.h"
#include "../FmAddressMarks.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::Tycom {
namespace {
Decoding::LocalizedText state(std::optional<bool> valid) {
    return Decoding::message(valid ? (*valid ? Constants::ValidKey : Constants::InvalidKey) : Constants::UnavailableKey);
}
std::string hexByte(std::uint8_t value) {
    return {Constants::HexDigits[value >> Constants::NibbleBits], Constants::HexDigits[value & Constants::NibbleMask]};
}
std::vector<Decoding::LocalizedText> identity(const Header& header) {
    return {Decoding::literal(std::to_string(header.cylinder)), Decoding::literal(std::to_string(header.sector))};
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
        if (!Flux::matchBytes(bits, offset, FmAddressMarks::forMark(HeaderMark))) continue;
        const auto header = decodeHeader(bits, offset, token);
        if (!header) {
            result.structures.push_back({Decoding::StructureKind::FormatHeader, std::uint64_t(offset),
                MarkBits, Decoding::message(TruncatedHeaderKey)});
            offset += MarkBits - 1; continue;
        }
        if (!header->crcValid) {
            auto args = identity(*header); args.push_back(state(false));
            result.structures.push_back({Decoding::StructureKind::FormatHeader, std::uint64_t(offset),
                HeaderBits, Decoding::message(InvalidHeaderKey, std::move(args))});
            offset += MarkBits - 1; continue;
        }
        result.decodedBytes.push_back(header->cylinder);
        result.decodedBytes.push_back(header->sector);
        const auto mark = findNextDataMark(bits, offset + HeaderBits, SearchBits, token);
        const auto data = mark ? decodeData(bits, *mark, token) : std::optional<Data>{};
        if (mark) {
            paired.insert(mark->offset);
            if (!data)
                result.structures.push_back({Decoding::StructureKind::FormatData, std::uint64_t(mark->offset),
                    MarkBits, Decoding::message(TruncatedDataKey, {Decoding::literal(hexByte(mark->mark))})});
            else {
                result.decodedBytes.insert(result.decodedBytes.end(), data->payload.begin(), data->payload.end());
                auto args = identity(*header);
                args.push_back(Decoding::literal(hexByte(mark->mark))); args.push_back(state(data->crcValid));
                result.structures.push_back({Decoding::StructureKind::FormatData, std::uint64_t(mark->offset),
                    DataBits, Decoding::message(DataKey, std::move(args))});
            }
        }
        Decoding::Sector sector;
        sector.cylinder = header->cylinder; sector.head = LogicalHead; sector.number = header->sector;
        sector.sizeCode = SizeCode; sector.sizeBytes = SectorSize; sector.bitOffset = offset;
        if (data) { sector.integrityValid = data->crcValid; sector.data = data->payload; }
        result.sectors.push_back(std::move(sector));
        auto args = identity(*header);
        if (mark) args.push_back(Decoding::literal(hexByte(mark->mark)));
        args.push_back(state(true)); args.push_back(state(data ? std::optional<bool>(data->crcValid) : std::nullopt));
        result.structures.push_back({Decoding::StructureKind::FormatHeader, std::uint64_t(offset),
            HeaderBits, Decoding::message(mark ? HeaderKey : HeaderNoMarkKey, std::move(args))});
        offset += MarkBits - 1;
    }
    for (std::int64_t offset = 0; offset + MarkBits <= std::int64_t(bits.size()); ++offset) {
        checkCancellation(token);
        if (paired.contains(offset)) continue;
        for (auto mark = std::int32_t(FirstDataMark); mark <= LastDataMark; ++mark) {
            if (!Flux::matchBytes(bits, offset, FmAddressMarks::forMark(std::uint8_t(mark)))) continue;
            result.structures.push_back({Decoding::StructureKind::FormatData, std::uint64_t(offset),
                MarkBits, Decoding::message(UnpairedKey, {Decoding::literal(hexByte(std::uint8_t(mark)))})});
            offset += MarkBits - 1; break;
        }
    }
    std::stable_sort(result.structures.begin(), result.structures.end(),
        [](const auto& left, const auto& right) { return left.bitOffset < right.bitOffset; });
    result.confidence = Decoding::confidence(std::int32_t(result.sectors.size()),
        std::int32_t(result.structures.size()), ConfidenceWeight, ConfidenceDivisor);
    return result;
}
}

