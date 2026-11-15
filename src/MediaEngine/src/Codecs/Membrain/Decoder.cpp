#include "Mfm.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <unordered_set>

namespace RetroMediaLab::MediaEngine::Membrain {
namespace {
Decoding::LocalizedText state(std::optional<bool> valid) {
    return Decoding::message(valid ? (*valid ? Constants::ValidKey : Constants::InvalidKey)
        : Constants::UnavailableKey);
}
std::vector<Decoding::LocalizedText> identity(const Address& address) {
    return {Decoding::literal(std::to_string(address.cylinder)),
        Decoding::literal(std::to_string(address.head)), Decoding::literal(std::to_string(address.sector))};
}
std::string hexByte(std::uint8_t value) {
    return {Constants::HexDigits[value >> Constants::NibbleBits],
        Constants::HexDigits[value & Constants::NibbleMask]};
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
    for (std::int64_t offset = 0; offset + PatternBits <= std::int64_t(bits.size()); ++offset) {
        checkCancellation(token);
        if (!Flux::matchBytes(bits, offset, HeaderPattern)) continue;
        const auto header = decodeHeader(bits, offset, token);
        if (!header) {
            result.structures.push_back({Decoding::StructureKind::FormatHeader,
                std::uint64_t(offset), PatternBits, Decoding::message(TruncatedHeaderKey)});
            offset += PatternBits - 1;
            continue;
        }
        result.decodedBytes.insert(result.decodedBytes.end(), header->bytes.begin(), header->bytes.end());
        const auto dataOffset = findDataMark(bits, offset + SearchStart,
            std::min(std::int64_t(bits.size()), offset + SearchBits), token);
        const auto data = dataOffset < 0 ? std::optional<Data>{} : decodeData(bits, dataOffset, token);
        if (dataOffset >= 0) {
            paired.insert(dataOffset);
            if (!data)
                result.structures.push_back({Decoding::StructureKind::FormatData,
                    std::uint64_t(dataOffset), PatternBits, Decoding::message(TruncatedDataKey)});
            else {
                result.decodedBytes.insert(result.decodedBytes.end(), data->payload.begin(), data->payload.end());
                auto args = identity(header->address);
                args.push_back(Decoding::literal(hexByte(data->mark)));
                args.push_back(state(data->crcValid));
                result.structures.push_back({Decoding::StructureKind::FormatData,
                    std::uint64_t(dataOffset), DataBytes * EncodedByteBits,
                    Decoding::message(DataKey, std::move(args))});
            }
        }
        Decoding::Sector sector;
        sector.cylinder = header->address.cylinder;
        sector.head = header->address.head;
        sector.number = header->address.sector;
        sector.sizeCode = SectorSizeCode;
        sector.sizeBytes = SectorSize;
        sector.bitOffset = offset;
        if (!header->crcValid || (data && !data->crcValid)) sector.integrityValid = false;
        else if (data) sector.integrityValid = true;
        if (data) sector.data = data->payload;
        result.sectors.push_back(std::move(sector));
        auto args = identity(header->address);
        args.push_back(state(header->crcValid));
        args.push_back(state(data ? std::optional<bool>(data->crcValid) : std::nullopt));
        result.structures.push_back({Decoding::StructureKind::FormatHeader,
            std::uint64_t(offset), HeaderBits, Decoding::message(HeaderKey, std::move(args))});
        const auto end = data ? data->offset + DataBytes * EncodedByteBits : offset + HeaderBits;
        offset = std::max(offset + PatternBits - 1, end - 1);
    }
    for (std::int64_t offset = 0; offset + PatternBits <= std::int64_t(bits.size()); ++offset) {
        const auto found = findDataMark(bits, offset, bits.size(), token);
        if (found < 0) break;
        if (!paired.contains(found))
            result.structures.push_back({Decoding::StructureKind::FormatData,
                std::uint64_t(found), PatternBits, Decoding::message(UnpairedKey)});
        offset = found + PatternBits - 1;
    }
    result.confidence = Decoding::confidence(std::int32_t(result.sectors.size()),
        std::int32_t(result.structures.size()), ConfidenceWeight, ConfidenceDivisor);
    return result;
}
}

