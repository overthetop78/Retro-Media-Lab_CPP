#include "Fm.h"
#include "../Crc16.h"
#include "../FmAddressMarks.h"
#include "../../Models/Flux/BitReader.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Tycom {
std::optional<Header> decodeHeader(Flux::BitView bits, std::int64_t offset, std::stop_token token) {
    checkCancellation(token);
    if (offset < -Constants::MarkBits || offset > std::int64_t(bits.size())) return {};
    const auto bytes = Flux::fmBytes32(bits, offset + Constants::MarkBits, Constants::HeaderBytes, token);
    if (!bytes) return {};
    std::vector<std::uint8_t> checked{Constants::HeaderMark};
    checked.insert(checked.end(), bytes->begin(), bytes->end());
    return Header{offset, (*bytes)[0], (*bytes)[1], Crc16::compute(checked,
        Crc16::Constants::Ccitt, Crc16::Constants::AllBitsSet, token) == 0, *bytes};
}
std::optional<DataMark> findNextDataMark(Flux::BitView bits, std::int64_t start,
    std::int64_t maximumDistance, std::stop_token token) {
    checkCancellation(token);
    const auto last = std::int64_t(bits.size()) - Constants::MarkBits;
    if (start > last || maximumDistance < 0) return {};
    const auto first = std::max(std::int64_t(0), start);
    if (first > last || (start < 0 && start < -maximumDistance)) return {};
    const auto end = start < 0 ? std::min(last, start + maximumDistance)
        : maximumDistance >= last - start ? last : start + maximumDistance;
    for (auto offset = first; offset <= end; ++offset) {
        checkCancellation(token);
        if (Flux::matchBytes(bits, offset, FmAddressMarks::forMark(Constants::HeaderMark))) return {};
        for (auto mark = std::int32_t(Constants::FirstDataMark); mark <= Constants::LastDataMark; ++mark)
            if (Flux::matchBytes(bits, offset, FmAddressMarks::forMark(std::uint8_t(mark))))
                return DataMark{offset, std::uint8_t(mark)};
    }
    return {};
}
std::optional<Data> decodeData(Flux::BitView bits, DataMark mark, std::stop_token token) {
    const auto bytes = Flux::fmBytes32(bits, mark.offset, Constants::DataBytes, token);
    if (!bytes || bytes->front() != mark.mark) return {};
    return Data{mark, {bytes->begin() + 1, bytes->begin() + 1 + Constants::SectorSize},
        *bytes, Crc16::compute(*bytes, Crc16::Constants::Ccitt,
            Crc16::Constants::AllBitsSet, token) == 0};
}
}

