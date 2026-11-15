#include "Blocks.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>

namespace RetroMediaLab::MediaEngine::Arburg {
Flux::Bits encodeSystem(MediaFileSystems::Bytes values, std::stop_token token) {
    checkCancellation(token);
    Flux::Bits bits;
    for (auto value : values) {
        checkCancellation(token);
        for (int bit = 0; bit < Constants::BitsPerByte; ++bit) {
            bits.push_back(Constants::PrefixBit);
            const auto set = ((value >> bit) & 1) != 0;
            bits.push_back(set ? Constants::OneSecondBit : Constants::ZeroSecondBit);
            if (set) bits.push_back(Constants::OneThirdBit);
        }
    }
    return bits;
}

std::optional<SystemData> decodeSystem(Flux::BitView bits, std::int64_t start,
    std::int64_t count, std::stop_token token) {
    checkCancellation(token);
    if (count < 0 || count > Constants::MaximumSystemBytes)
        throw MediaError(DiagnosticCode::BitReadCount,
            {std::to_string(count), std::to_string(Constants::MaximumSystemBytes)});
    SystemData result{{}, start};
    if (count == 0) return result;
    if (start < 0 || std::uint64_t(start) > bits.size()) return {};
    if (std::uint64_t(count) > (bits.size() - std::size_t(start))
        / Constants::BitsPerByte / Constants::ZeroBits) return {};
    result.bytes.reserve(std::size_t(count));
    auto offset = start;
    for (std::int64_t index = 0; index < count; ++index) {
        checkCancellation(token);
        std::uint8_t value = 0;
        for (int bit = 0; bit < Constants::BitsPerByte; ++bit) {
            if (offset + Constants::ZeroBits > std::int64_t(bits.size())
                || bits[std::size_t(offset)] != Constants::PrefixBit) return {};
            if (bits[std::size_t(offset) + 1] == Constants::ZeroSecondBit)
                offset += Constants::ZeroBits;
            else {
                if (offset + Constants::OneBits > std::int64_t(bits.size())
                    || bits[std::size_t(offset) + 1] != Constants::OneSecondBit
                    || bits[std::size_t(offset) + Constants::OneThirdBitOffset] != Constants::OneThirdBit) return {};
                value |= std::uint8_t(1 << bit);
                offset += Constants::OneBits;
            }
        }
        result.bytes.push_back(value);
    }
    result.endOffset = offset;
    return result;
}
}

