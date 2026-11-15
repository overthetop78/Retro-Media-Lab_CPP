#include "Mfm.h"
#include "../../Models/Flux/BitReader.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::QdMo5 {
std::uint8_t checksum(std::uint8_t prefix, MediaFileSystems::Bytes bytes, std::stop_token token) {
    checkCancellation(token);
    auto result = prefix;
    for (auto value : bytes) { checkCancellation(token); result = std::uint8_t(result + value); }
    return result;
}
std::optional<Header> decodeHeader(Flux::BitView bits, std::int64_t offset, std::stop_token token) {
    checkCancellation(token);
    if (offset < -Constants::MarkBits || offset > std::int64_t(bits.size())) return {};
    const auto bytes = Flux::mfmBytes(bits, offset + Constants::MarkBits, Constants::HeaderTailBytes, token);
    if (!bytes) return {};
    return Header{offset, ((*bytes)[0] << Constants::BitsPerByte) | (*bytes)[1],
        {bytes->begin() + Constants::SectorNumberBytes, bytes->end()}};
}
std::optional<Data> decodeData(Flux::BitView bits, std::int64_t offset, std::stop_token token) {
    checkCancellation(token);
    if (offset < -Constants::PreambleBits || offset > std::int64_t(bits.size())) return {};
    const auto bytes = Flux::mfmBytes(bits, offset + Constants::PreambleBits, Constants::DataBytes, token);
    if (!bytes) return {};
    const auto prefix = bytes->front(), stored = bytes->back();
    std::vector<std::uint8_t> payload(bytes->begin() + 1, bytes->begin() + 1 + Constants::SectorSize);
    const auto valid = checksum(prefix, payload, token) == stored;
    return Data{offset, prefix, std::move(payload), stored, valid};
}
std::int64_t findNextData(Flux::BitView bits, std::int64_t start, std::int64_t maximumDistance,
    std::stop_token token) {
    checkCancellation(token);
    const auto last = std::int64_t(bits.size()) - Constants::MarkBits;
    if (start > last || maximumDistance < 0) return -1;
    // Avoid start + distance overflow, including starts outside the source.
    const auto first = std::max(std::int64_t(0), start);
    if (first > last) return -1;
    if (start < 0 && start < -maximumDistance) return -1;
    const auto end = maximumDistance >= last - first ? last : first + maximumDistance;
    // A negative start spends part of its original search window before bit zero.
    const auto effectiveEnd = start < 0 ? std::min(last, start + maximumDistance) : end;
    for (auto offset = first; offset <= effectiveEnd; ++offset) {
        checkCancellation(token);
        if (!Flux::matchBytes(bits, offset, Constants::Preamble)) continue;
        const auto mark = Flux::mfmByte(bits, offset + Constants::PreambleBits);
        if (!mark) continue;
        return *mark == Constants::HeaderAddressMark ? -1 : offset;
    }
    return -1;
}
}

