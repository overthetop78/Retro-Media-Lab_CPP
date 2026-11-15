#include "Mfm.h"
#include "../Crc16.h"
#include "../../Models/Flux/BitReader.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>

namespace RetroMediaLab::MediaEngine::Membrain {
namespace {
bool dataMark(std::uint8_t mark) {
    return mark >= Constants::FirstDataMark && mark <= Constants::LastDataMark;
}
bool validCrc(MediaFileSystems::Bytes bytes, std::stop_token token) {
    return Crc16::compute(bytes, Crc16::Constants::Ibm, Crc16::Constants::Zero, token) == 0;
}
}
std::optional<Header> decodeHeader(Flux::BitView bits, std::int64_t offset, std::stop_token token) {
    const auto bytes = Flux::mfmBytes(bits, offset, Constants::HeaderBytes, token);
    if (!bytes || (*bytes)[Constants::HeaderMarkOffset] != Constants::HeaderMark) return {};
    return Header{offset, unpack((*bytes)[Constants::CylinderHighOffset], (*bytes)[Constants::PackedOffset]),
        validCrc(*bytes, token), *bytes};
}

std::int64_t findDataMark(Flux::BitView bits, std::int64_t start, std::int64_t end, std::stop_token token) {
    checkCancellation(token);
    const auto limit = std::min(end, std::int64_t(bits.size()));
    if (limit < Constants::PatternBits) return -1;
    for (auto offset = std::max(std::int64_t(0), start); offset <= limit - Constants::PatternBits; ++offset) {
        checkCancellation(token);
        if (!Flux::match(bits, offset, Constants::EncodedSync)) continue;
        const auto mark = Flux::mfmByte(bits, offset + Constants::EncodedByteBits);
        if (mark && dataMark(*mark)) return offset;
    }
    return -1;
}

std::optional<Data> decodeData(Flux::BitView bits, std::int64_t offset, std::stop_token token) {
    const auto bytes = Flux::mfmBytes(bits, offset, Constants::DataBytes, token);
    if (!bytes || !dataMark((*bytes)[Constants::HeaderMarkOffset])) return {};
    return Data{offset, (*bytes)[Constants::HeaderMarkOffset],
        {bytes->begin() + Constants::DataPrefixBytes,
            bytes->begin() + Constants::DataPrefixBytes + Constants::SectorSize},
        *bytes, validCrc(*bytes, token)};
}
}

